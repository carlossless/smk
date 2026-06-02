#include "indicators.h"
#include "kbdef.h"
#include "pwm.h"
#include "settings.h"
#include "keyboard.h"
#ifdef RF_ENABLED
#    include "rf_controller.h"
#endif

// TODO: move these defines out
#define PWM_CLK_DIV         0b010 // PWM_CLK = SYS_CLK / 4
#define PWM_SS_BIT          (1 << 3)
#define PWM_MOD_BIT         (1 << 4)
#define PWM_INT_ENABLE_BIT  (1 << 6)
#define PWM_MODE_ENABLE_BIT (1 << 7)

// The LED matrix follows the key matrix dimensions, so a differently-sized RGB board
// only needs the right MATRIX_ROWS/MATRIX_COLS (kbdef.h), its geometry parameters in
// meson.build, plus its own sink/column wiring below.
#define LED_ROWS MATRIX_ROWS
#define LED_COLS MATRIX_COLS

// The underglow ("user") LEDs are wired as an extra sink group, scanned as one more
// row after the key matrix. They mirror the bottom key row so they animate together
// with whatever effect is active.
#define LED_UL_ROW    LED_ROWS
#define LED_SCAN_ROWS (LED_ROWS + 1)

// Defaults / clamps for user_settings.led_speed (phase increment per ~73 Hz sweep).
#define LED_SPEED_DEFAULT    4
#define LED_UL_SPEED_DEFAULT 1 // underglow defaults slow - it's ambient
#define LED_SPEED_MIN        1
#define LED_SPEED_MAX        16

// Underglow LEDs at or below this column display the status colour (caps / conn mode).
// LEDs at columns >= UL_STATUS_COLS form the right half, used by the battery indicator.
#define UL_STATUS_COLS (LED_COLS / 2)

// How long the FN+[ "flash battery" momentary indicator stays on, measured in
// full UL sweeps. One sweep happens at ~73 Hz so 200 ≈ 2.7 s.
#define BAT_FLASH_SWEEPS 200

// Brightness step for one Fn+Up / Fn+Down press (out of 256 levels).
#define LED_BRIGHTNESS_DEFAULT 255
#define LED_BRIGHTNESS_STEP    32

// 8-bit framebuffer brightness -> column PWM DUTY2 register.
// PWM hardware: DUTY1=0 transitions output LOW at start of period, then
// DUTY2 transitions it back HIGH. LED conducts while output is LOW.
// LED-on-time = DUTY2 cycles, so direct fb → DUTY2 mapping gives
// fb=0 → off, fb=255 → ~99.6% on.
//
// (The previous inversion-attempt mapped fb=0 to DUTY2=0xFF which made
// supposedly-off subframes glow at MAX brightness — the USB-mode side
// light went blue instead of yellow because the "off" blue subframe was
// actually being driven at full duty.)
#define LED_DUTY(v) (uint16_t)(v)

// Scale an 8-bit channel by the current brightness (main and underglow are independent).
#define SCALE_BRI(v)    (uint8_t)(((uint16_t)(uint8_t)(v) * user_settings.led_brightness) >> 8)
#define SCALE_UL_BRI(v) (uint8_t)(((uint16_t)(uint8_t)(v) * user_settings.ul_brightness) >> 8)

typedef enum {
    FX_RADIAL = 0, // rainbow rings radiating from the centre
    FX_HORIZONTAL, // rainbow sweeping across columns
    FX_VERTICAL,   // rainbow sweeping across rows
    FX_SOLID,      // static solid white (no animation)
    FX_COUNT
} led_effect_t;

// The cycle key steps OFF -> each effect -> OFF -> ...; FX_OFF is the dark state.
#define FX_OFF FX_COUNT

// Per-device geometry, baked into ROM at build time by utils/led_geometry_gen.c and
// included here as const __code tables (radial_index, col_hue, row_hue). Costs no RAM
// and no runtime computation. The generated header is selected per board via meson.
#include LED_GEOMETRY_HEADER
_Static_assert(LED_GEOMETRY_ROWS == LED_ROWS && LED_GEOMETRY_COLS == LED_COLS, "generated LED geometry size does not match the key matrix");

// Per-LED RGB framebuffer for the main key matrix, indexed [row][color][col].
// Kept entirely in __xdata so the scarce internal RAM stays untouched.
static __xdata uint8_t led_fb[LED_ROWS][3][LED_COLS];

// Separate framebuffer for the underglow ("user") LEDs, since they animate
// independently of the main backlight.
static __xdata uint8_t led_ul_fb[3][LED_COLS];

// LED scan cursor, advanced one (row,color) substep per PWM ISR. Decoupled from
// the key-matrix column scan (current_step). Also __xdata to avoid internal RAM.
static __xdata uint8_t led_row;
static __xdata uint8_t led_color;

// Animation state. The framebuffers are regenerated one LED per ISR cycling through
// the main rows then the underglow row (regen_row/regen_col). led_phase / ul_phase
// shift their respective rainbow each time their portion of the sweep completes.
static __xdata uint8_t led_phase;
static __xdata uint8_t ul_phase;
static __xdata uint8_t regen_row;
static __xdata uint8_t regen_col;

// FN+[ momentary battery indicator: counts down once per UL sweep; while non-zero
// the right-side UL LEDs show the battery colour regardless of the persistent
// `user_settings.battery_indicator_on` flag.
static __xdata uint8_t battery_flash_sweeps;

// Free-running counter incremented once per UL sweep (~73 Hz). Used to drive
// the unpaired (fast blink) / disconnected (slow breath) status indicator
// on the left-side UL LEDs.
static __xdata uint8_t status_pulse_counter;

// Quarter-sine LUT used to drive the disconnected "breathing" effect. 32
// entries, 0..255 amplitude over 0..π/2. Mirrored at runtime to produce a
// full breath cycle in 128 sweeps (~1.75 s, ~0.6 Hz at the 73 Hz UL rate).
static __code const uint8_t breath_lut[32] = {
    0,  13, 26, 39,  51,  64,  76,  88,  100, 112, 124, 135, 146, 156, 166, 176,
    185, 194, 202, 210, 217, 223, 229, 234, 239, 244, 247, 251, 253, 254, 255, 255
};

void        indicators_pwm_enable();
void        indicators_pwm_disable();
// (now public — declared in indicators.h)
static void led_regen_one();
static void led_enable_sink();
static void led_set_columns();

// Sets every field of user_settings to its factory default value.
void indicators_apply_defaults()
{
    user_settings.led_effect     = FX_RADIAL;
    user_settings.led_brightness = LED_BRIGHTNESS_DEFAULT;
    user_settings.led_speed      = LED_SPEED_DEFAULT;
    user_settings.ul_effect      = FX_RADIAL;
    user_settings.ul_brightness  = LED_BRIGHTNESS_DEFAULT;
    user_settings.ul_speed       = LED_UL_SPEED_DEFAULT;
    user_settings.battery_indicator_on = 0;
#ifdef RF_ENABLED
    user_settings.rf_link = RF_MODE_2_4G;
#endif
}

void indicators_battery_flash()
{
    battery_flash_sweeps = BAT_FLASH_SWEEPS;
}

void indicators_battery_on()
{
    user_settings.battery_indicator_on = 1;
    settings_mark_dirty();
}

void indicators_battery_off()
{
    user_settings.battery_indicator_on = 0;
    settings_mark_dirty();
}

// Factory reset: restore defaults and persist them so the next boot loads them too.
void indicators_factory_reset()
{
    indicators_apply_defaults();
    settings_mark_dirty();
}

// Clamp any LED-side fields in user_settings to their valid ranges. Called
// from main() after settings_load() restores the on-flash record (which
// may be stale, from a struct-changing build).
void indicators_validate_settings()
{
    if (user_settings.led_effect > FX_OFF) {
        user_settings.led_effect = FX_OFF;
    }
    if (user_settings.led_speed < LED_SPEED_MIN) {
        user_settings.led_speed = LED_SPEED_MIN;
    }
    if (user_settings.led_speed > LED_SPEED_MAX) {
        user_settings.led_speed = LED_SPEED_MAX;
    }
    if (user_settings.ul_effect > FX_OFF) {
        user_settings.ul_effect = FX_OFF;
    }
    if (user_settings.ul_speed < LED_SPEED_MIN) {
        user_settings.ul_speed = LED_SPEED_MIN;
    }
    if (user_settings.ul_speed > LED_SPEED_MAX) {
        user_settings.ul_speed = LED_SPEED_MAX;
    }
}

void indicators_start()
{
    led_row   = 0;
    led_color = 0;

    led_phase = 0;
    ul_phase  = 0;
    regen_row = 0;
    regen_col = 0;

    indicators_pwm_enable();
}

void indicators_next_effect()
{
    // OFF -> FX_RADIAL -> ... -> FX_SOLID -> OFF -> ...
    if (++user_settings.led_effect > FX_OFF) {
        user_settings.led_effect = 0;
    }

    settings_mark_dirty();
}

void indicators_prev_effect()
{
    // OFF <- FX_RADIAL <- ... <- FX_SOLID <- OFF <- ...
    if (user_settings.led_effect == 0) {
        user_settings.led_effect = FX_OFF;
    } else {
        user_settings.led_effect--;
    }

    settings_mark_dirty();
}

void indicators_brightness_up()
{
    if (user_settings.led_brightness > (uint8_t)(255 - LED_BRIGHTNESS_STEP)) {
        user_settings.led_brightness = 255;
    } else {
        user_settings.led_brightness = (uint8_t)(user_settings.led_brightness + LED_BRIGHTNESS_STEP);
    }
    settings_mark_dirty();
}

void indicators_brightness_down()
{
    if (user_settings.led_brightness < LED_BRIGHTNESS_STEP) {
        user_settings.led_brightness = 0;
    } else {
        user_settings.led_brightness = (uint8_t)(user_settings.led_brightness - LED_BRIGHTNESS_STEP);
    }
    settings_mark_dirty();
}

void indicators_speed_up()
{
    if (user_settings.led_speed < LED_SPEED_MAX) {
        user_settings.led_speed++;
    }

    settings_mark_dirty();
}

void indicators_speed_down()
{
    if (user_settings.led_speed > LED_SPEED_MIN) {
        user_settings.led_speed--;
    }

    settings_mark_dirty();
}

// Underglow / "user-light" controls. Identical logic to the main backlight ones,
// but operating on the independent ul_* fields of user_settings.

void indicators_ul_next_effect()
{
    if (++user_settings.ul_effect > FX_OFF) {
        user_settings.ul_effect = 0;
    }
    settings_mark_dirty();
}

void indicators_ul_prev_effect()
{
    if (user_settings.ul_effect == 0) {
        user_settings.ul_effect = FX_OFF;
    } else {
        user_settings.ul_effect--;
    }
    settings_mark_dirty();
}

void indicators_ul_brightness_up()
{
    if (user_settings.ul_brightness > (uint8_t)(255 - LED_BRIGHTNESS_STEP)) {
        user_settings.ul_brightness = 255;
    } else {
        user_settings.ul_brightness = (uint8_t)(user_settings.ul_brightness + LED_BRIGHTNESS_STEP);
    }
    settings_mark_dirty();
}

void indicators_ul_brightness_down()
{
    if (user_settings.ul_brightness < LED_BRIGHTNESS_STEP) {
        user_settings.ul_brightness = 0;
    } else {
        user_settings.ul_brightness = (uint8_t)(user_settings.ul_brightness - LED_BRIGHTNESS_STEP);
    }
    settings_mark_dirty();
}

void indicators_ul_speed_up()
{
    if (user_settings.ul_speed < LED_SPEED_MAX) {
        user_settings.ul_speed++;
    }
    settings_mark_dirty();
}

void indicators_ul_speed_down()
{
    if (user_settings.ul_speed > LED_SPEED_MIN) {
        user_settings.ul_speed--;
    }
    settings_mark_dirty();
}

void indicators_pre_update()
{
    // Drop every RGB row sink before turning on the one we want. Stock
    // does this every tick (only one sink is ever high in steady state).
    P0 &= ~(RGB_R2R_P0_2 | RGB_R0B_P0_3 | RGB_R0R_P0_4);
    P1 &= ~(RGB_ULR_P1_1 | RGB_ULG_P1_2 | RGB_ULB_P1_3);
    P4 &= ~(RGB_R4B_P4_3 | RGB_R4R_P4_4 | RGB_R3R_P4_5 | RGB_R3B_P4_6);
    P5 &= ~(RGB_R2B_P5_7);
    P6 &= ~(RGB_R0G_P6_1 | RGB_R1G_P6_2 | RGB_R2G_P6_3 | RGB_R3G_P6_4 | RGB_R4G_P6_5 | RGB_R1B_P6_6 | RGB_R1R_P6_7);

    // PWM channels stay enabled across ticks — stock never tears down the
    // PWM hardware between substeps. Disabling+re-enabling around every
    // duty write made the column GPIO latch leak through (LEDs full-on
    // during the disable window) and produced visible flicker.
    // indicators_pwm_disable() removed intentionally.
}

bool indicators_update_step(keyboard_state_t *keyboard, uint8_t current_step)
{
    keyboard;
    current_step;

    // Regenerate one LED of the framebuffer per ISR (cheap), spreading the rainbow
    // computation over many interrupts instead of one expensive burst.
    led_regen_one();

    // Main rows scan only when the main effect is on. The UL row always scans so the
    // left-side status indicator stays visible regardless of UL effect.
    //
    // Order matters for flicker: write the 16 column duties BEFORE enabling
    // the row sink. Stock fw does duties-first / sink-last so the new row
    // never gets driven with the previous row's framebuffer. Our previous
    // order (sink first) lit each row with the prior row's values for a
    // few µs every substep — visible as flicker / colour bleed.
    if (led_row < LED_ROWS) {
        if (user_settings.led_effect < FX_OFF) {
            led_set_columns();
            led_enable_sink();
        }
    } else {
        led_set_columns();
        led_enable_sink();
    }

    bool frame_wrapped = false;
    if (++led_color >= 3) {
        led_color = 0;
        if (++led_row >= LED_SCAN_ROWS) {
            led_row        = 0;
            frame_wrapped  = true;
        }
    }

    return frame_wrapped;
}

void indicators_post_update()
{
    // Acknowledge the PWM0 period-end flag in case anything ever turns
    // PWM0IE on. PWM channels themselves stay enabled — see comment in
    // indicators_pre_update.
    PWM00CON &= ~(1 << 5);
}

static void led_regen_one()
{
    if (regen_row < LED_ROWS) {
        // -------- MAIN backlight cell --------
        if (user_settings.led_effect == FX_SOLID) {
            led_fb[regen_row][0][regen_col] = user_settings.led_brightness;
            led_fb[regen_row][1][regen_col] = user_settings.led_brightness;
            led_fb[regen_row][2][regen_col] = user_settings.led_brightness;
        } else if (user_settings.led_effect < FX_OFF) {
            uint8_t h;
            switch (user_settings.led_effect) {
                case FX_HORIZONTAL:
                    h = (uint8_t)(col_hue[regen_col] + led_phase);
                    break;
                case FX_VERTICAL:
                    h = (uint8_t)(row_hue[regen_row] + led_phase);
                    break;
                case FX_RADIAL:
                default:
                    h = (uint8_t)(radial_index[regen_row][regen_col] + led_phase);
                    break;
            }

            // Full-saturation colour wheel (h -> R->B->G->R), scaled by brightness.
            if (h < 85) {
                led_fb[regen_row][0][regen_col] = SCALE_BRI(255 - h * 3);
                led_fb[regen_row][1][regen_col] = 0;
                led_fb[regen_row][2][regen_col] = SCALE_BRI(h * 3);
            } else if (h < 170) {
                h                               = (uint8_t)(h - 85);
                led_fb[regen_row][0][regen_col] = 0;
                led_fb[regen_row][1][regen_col] = SCALE_BRI(h * 3);
                led_fb[regen_row][2][regen_col] = SCALE_BRI(255 - h * 3);
            } else {
                h                               = (uint8_t)(h - 170);
                led_fb[regen_row][0][regen_col] = SCALE_BRI(h * 3);
                led_fb[regen_row][1][regen_col] = SCALE_BRI(255 - h * 3);
                led_fb[regen_row][2][regen_col] = 0;
            }
        }
        // if main is off, leave led_fb stale (the scanner skips that section)
    } else {
        // -------- UNDERGLOW cell --------
        if (user_settings.ul_effect == FX_SOLID) {
            led_ul_fb[0][regen_col] = user_settings.ul_brightness;
            led_ul_fb[1][regen_col] = user_settings.ul_brightness;
            led_ul_fb[2][regen_col] = user_settings.ul_brightness;
        } else if (user_settings.ul_effect < FX_OFF) {
            uint8_t h;
            switch (user_settings.ul_effect) {
                case FX_HORIZONTAL:
                    h = (uint8_t)(col_hue[regen_col] + ul_phase);
                    break;
                case FX_VERTICAL:
                    // UL is one strip, so vertical = whole strip cycling together
                    h = ul_phase;
                    break;
                case FX_RADIAL:
                default: {
                    // distance from the strip centre, scaled so the wheel spans
                    // from centre to either end
                    uint8_t r = (regen_col < (LED_COLS / 2)) ? (uint8_t)((LED_COLS / 2 - 1) - regen_col) : (uint8_t)(regen_col - LED_COLS / 2);
                    h         = (uint8_t)((uint8_t)(r * 36) + ul_phase);
                    break;
                }
            }

            if (h < 85) {
                led_ul_fb[0][regen_col] = SCALE_UL_BRI(255 - h * 3);
                led_ul_fb[1][regen_col] = 0;
                led_ul_fb[2][regen_col] = SCALE_UL_BRI(h * 3);
            } else if (h < 170) {
                h                       = (uint8_t)(h - 85);
                led_ul_fb[0][regen_col] = 0;
                led_ul_fb[1][regen_col] = SCALE_UL_BRI(h * 3);
                led_ul_fb[2][regen_col] = SCALE_UL_BRI(255 - h * 3);
            } else {
                h                       = (uint8_t)(h - 170);
                led_ul_fb[0][regen_col] = SCALE_UL_BRI(h * 3);
                led_ul_fb[1][regen_col] = SCALE_UL_BRI(255 - h * 3);
                led_ul_fb[2][regen_col] = 0;
            }
        } else {
            // UL off: clear so stale effect doesn't leak (the UL row always scans for status)
            led_ul_fb[0][regen_col] = 0;
            led_ul_fb[1][regen_col] = 0;
            led_ul_fb[2][regen_col] = 0;
        }

        // Status indicator on the left-side UL LEDs. Overrides the UL effect so the
        // mode is visible regardless of UL state.
        //   caps lock      -> grayish green (highest priority)
        //   wired (USB)    -> yellow
        //   2.4G wireless  -> green
        //   bluetooth      -> blue
        if (regen_col < UL_STATUS_COLS) {
            if (keyboard_state.led_state & (1 << 1)) {
                led_ul_fb[0][regen_col] = SCALE_UL_BRI(50);
                led_ul_fb[1][regen_col] = SCALE_UL_BRI(120);
                led_ul_fb[2][regen_col] = SCALE_UL_BRI(50);
            } else if (CONN_MODE_SWITCH) {
                led_ul_fb[0][regen_col] = SCALE_UL_BRI(255);
                led_ul_fb[1][regen_col] = SCALE_UL_BRI(180);
                led_ul_fb[2][regen_col] = 0;
#ifdef RF_ENABLED
            } else {
                // Pick base colour: 2.4G = green, BT = blue.
                uint8_t r = 0, g = 0, b = 0;
                if (keyboard_state.rf_link == RF_MODE_2_4G) {
                    g = 255;
                } else {
                    b = 255;
                }

                // Modulate brightness on RF connection state.
                //   !paired              -> fast blink (~9 Hz, 50% duty)
                //   paired, !connected   -> smooth breathing (~1.1 Hz)
                //   paired,  connected   -> solid (status_scale = 255)
                uint8_t status_scale = 255;
                if (!keyboard_state.paired) {
                    // Toggle every 4 counts (~9 Hz @ 73 Hz sweep rate).
                    status_scale = (status_pulse_counter & 0x04) ? 255 : 0;
                } else if (!keyboard_state.connected) {
                    // 64-count cycle (~0.87 s, ~1.1 Hz) using full 32-entry LUT
                    // with a triangular fold at p=31/32. Was 128-count / ~0.6 Hz.
                    uint8_t p   = status_pulse_counter & 0x3F;       // 0..63
                    uint8_t idx = (p & 0x20) ? (uint8_t)(0x3F - p)
                                              : p;                    // 0..31, mirrored
                    status_scale = breath_lut[idx];
                }
                r = (uint8_t)(((uint16_t)r * status_scale) >> 8);
                g = (uint8_t)(((uint16_t)g * status_scale) >> 8);
                b = (uint8_t)(((uint16_t)b * status_scale) >> 8);

                led_ul_fb[0][regen_col] = SCALE_UL_BRI(r);
                led_ul_fb[1][regen_col] = SCALE_UL_BRI(g);
                led_ul_fb[2][regen_col] = SCALE_UL_BRI(b);
#else
            } else {
                led_ul_fb[0][regen_col] = 0;
                led_ul_fb[1][regen_col] = 0;
                led_ul_fb[2][regen_col] = 0;
#endif
            }
        }
#ifdef RF_ENABLED
        // Right-side UL battery indicator. Active either continuously (FN+]
        // sets user_settings.battery_indicator_on) or for a brief flash on
        // FN+[ (battery_flash_sweeps counts down per sweep). Colour mapping
        // for battery_level (0..7, ~14% per step):
        //   low_power flag OR level <= 1  -> red    (<20%)
        //   level >= 6                    -> green  (>80%)
        //   else                          -> yellow (20-80%)
        else if (user_settings.battery_indicator_on || battery_flash_sweeps) {
            uint8_t r, g, b;
            if (keyboard_state.low_power || keyboard_state.battery_level <= 1) {
                r = 255; g = 0;   b = 0;
            } else if (keyboard_state.battery_level >= 6) {
                r = 0;   g = 255; b = 0;
            } else {
                r = 255; g = 180; b = 0;
            }
            led_ul_fb[0][regen_col] = SCALE_UL_BRI(r);
            led_ul_fb[1][regen_col] = SCALE_UL_BRI(g);
            led_ul_fb[2][regen_col] = SCALE_UL_BRI(b);
        }
#endif
    }

    // Cursor advance: main rows 0..LED_ROWS-1 then the UL row, then back to 0.
    // Each region bumps its own phase when its sweep completes.
    if (++regen_col >= LED_COLS) {
        regen_col = 0;
        regen_row++;
        if (regen_row == LED_UL_ROW) {
            // just finished the main sweep
            led_phase = (uint8_t)(led_phase + user_settings.led_speed);
        } else if (regen_row >= LED_SCAN_ROWS) {
            // just finished the UL sweep
            ul_phase  = (uint8_t)(ul_phase + user_settings.ul_speed);
            regen_row = 0;
            if (battery_flash_sweeps) {
                battery_flash_sweeps--;
            }
            status_pulse_counter++;
        }
    }
}

static void led_enable_sink()
{
    switch (led_row) {
        case 0:
            if (led_color == 0)
                RGB_R0R = 1;
            else if (led_color == 1)
                RGB_R0G = 1;
            else
                RGB_R0B = 1;
            break;
        case 1:
            if (led_color == 0)
                RGB_R1R = 1;
            else if (led_color == 1)
                RGB_R1G = 1;
            else
                RGB_R1B = 1;
            break;
        case 2:
            if (led_color == 0)
                RGB_R2R = 1;
            else if (led_color == 1)
                RGB_R2G = 1;
            else
                RGB_R2B = 1;
            break;
        case 3:
            if (led_color == 0)
                RGB_R3R = 1;
            else if (led_color == 1)
                RGB_R3G = 1;
            else
                RGB_R3B = 1;
            break;
        case 4:
            if (led_color == 0)
                RGB_R4R = 1;
            else if (led_color == 1)
                RGB_R4G = 1;
            else
                RGB_R4B = 1;
            break;
        case LED_UL_ROW:
            if (led_color == 0)
                RGB_ULR = 1;
            else if (led_color == 1)
                RGB_ULG = 1;
            else
                RGB_ULB = 1;
            break;
    }
}

static void led_set_columns()
{
    // underglow uses its own framebuffer; key rows use the main one
    __xdata uint8_t *fb = (led_row == LED_UL_ROW) ? led_ul_fb[led_color] : led_fb[led_row][led_color];

    SET_PWM_DUTY_2(LED_PWM_C0, LED_DUTY(fb[0]));
    SET_PWM_DUTY_2(LED_PWM_C1, LED_DUTY(fb[1]));
    SET_PWM_DUTY_2(LED_PWM_C2, LED_DUTY(fb[2]));
    SET_PWM_DUTY_2(LED_PWM_C3, LED_DUTY(fb[3]));
    SET_PWM_DUTY_2(LED_PWM_C4, LED_DUTY(fb[4]));
    SET_PWM_DUTY_2(LED_PWM_C5, LED_DUTY(fb[5]));
    SET_PWM_DUTY_2(LED_PWM_C6, LED_DUTY(fb[6]));
    SET_PWM_DUTY_2(LED_PWM_C7, LED_DUTY(fb[7]));
    SET_PWM_DUTY_2(LED_PWM_C8, LED_DUTY(fb[8]));
    SET_PWM_DUTY_2(LED_PWM_C9, LED_DUTY(fb[9]));
    SET_PWM_DUTY_2(LED_PWM_C10, LED_DUTY(fb[10]));
    SET_PWM_DUTY_2(LED_PWM_C11, LED_DUTY(fb[11]));
    SET_PWM_DUTY_2(LED_PWM_C12, LED_DUTY(fb[12]));
    SET_PWM_DUTY_2(LED_PWM_C13, LED_DUTY(fb[13]));
    SET_PWM_DUTY_2(LED_PWM_C14, LED_DUTY(fb[14]));
    SET_PWM_DUTY_2(LED_PWM_C15, LED_DUTY(fb[15]));
}

void indicators_pwm_enable()
{
    // Stock fw led_pwm_init_all_modules values, verbatim:
    //
    //   PWM00CON = 0x8a (EN=1, IE=0, MOD=0, SS=1, CK=010)
    //   PWM01..05CON = 0x08 (per-channel SS=1 only)
    //   PWM10CON = 0x8a, PWM11..15CON = 0x08
    //   PWM20CON = 0x82 (EN=1, SS=0 — separate-output mode), PWM21..23CON = 0
    //   PWM24/25CON = 0x08
    //   PWM40CON = 0x8a, PWM41/42CON = 0x08
    //
    // Per-bank PWM IE = 0 across the board — no PWM ISRs fire.
    PWM00CON = (uint8_t)(PWM_MODE_ENABLE_BIT | PWM_SS_BIT | PWM_CLK_DIV);
    PWM01CON = PWM_SS_BIT;
    PWM02CON = PWM_SS_BIT;
    PWM03CON = PWM_SS_BIT;
    PWM04CON = PWM_SS_BIT;
    PWM05CON = PWM_SS_BIT;

    PWM10CON = (uint8_t)(PWM_MODE_ENABLE_BIT | PWM_SS_BIT | PWM_CLK_DIV);
    PWM11CON = PWM_SS_BIT;
    PWM12CON = PWM_SS_BIT;
    PWM13CON = PWM_SS_BIT;
    PWM14CON = PWM_SS_BIT;
    PWM15CON = PWM_SS_BIT;

    // PWM2 bank: separate-output mode (no SS bit on the master), 21..23 fully
    // off (unused channels — these pins are matrix col P1.1..1.3 in the
    // stock layout; SS=0 keeps the PWM output detached so the GPIO drives).
    PWM20CON = (uint8_t)(PWM_MODE_ENABLE_BIT | PWM_CLK_DIV);
    PWM21CON = 0;
    PWM22CON = 0;
    PWM23CON = 0;
    PWM24CON = PWM_SS_BIT;
    PWM25CON = PWM_SS_BIT;

    PWM40CON = (uint8_t)(PWM_MODE_ENABLE_BIT | PWM_SS_BIT | PWM_CLK_DIV);
    PWM41CON = PWM_SS_BIT;
    PWM42CON = PWM_SS_BIT;
}

void indicators_pwm_disable()
{
    // TODO: try abstracting individual banks away
    PWM00CON = (uint8_t)(PWM_CLK_DIV);
    PWM01CON = 0;
    PWM02CON = 0;
    PWM03CON = 0;
    PWM04CON = 0;
    PWM05CON = 0;

    PWM10CON = (uint8_t)(PWM_CLK_DIV);
    PWM11CON = 0;
    PWM12CON = 0;
    PWM13CON = 0;
    PWM14CON = 0;
    PWM15CON = 0;

    PWM20CON = (uint8_t)(PWM_CLK_DIV);
    // PWM24CON = 0;
    PWM25CON = 0;

    PWM40CON = (uint8_t)(PWM_CLK_DIV);
    PWM41CON = 0;
    PWM42CON = 0;
}
