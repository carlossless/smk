#include "indicators.h"
#include "kbdef.h"
#include "pwm.h"
#include "settings.h"
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
#define UL_STATUS_COLS (LED_COLS / 2)

// Brightness step for one Fn+Up / Fn+Down press (out of 256 levels).
#define LED_BRIGHTNESS_DEFAULT 255
#define LED_BRIGHTNESS_STEP    32

// 8-bit framebuffer brightness -> 10-bit column PWM duty (0 = full on, 0x400 = off)
#define LED_DUTY(v) (uint16_t)(0x0400u - ((uint16_t)(v) << 2))

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

void        indicators_pwm_enable();
void        indicators_pwm_disable();
static void apply_defaults();
static void led_regen_one();
static void led_enable_sink();
static void led_set_columns();

// Sets every field of user_settings to its factory default value.
static void apply_defaults()
{
    user_settings.led_effect     = FX_RADIAL;
    user_settings.led_brightness = LED_BRIGHTNESS_DEFAULT;
    user_settings.led_speed      = LED_SPEED_DEFAULT;
    user_settings.ul_effect      = FX_RADIAL;
    user_settings.ul_brightness  = LED_BRIGHTNESS_DEFAULT;
    user_settings.ul_speed       = LED_UL_SPEED_DEFAULT;
}

// Factory reset: restore defaults and persist them so the next boot loads them too.
void indicators_factory_reset()
{
    apply_defaults();
    settings_save();
}

void indicators_start()
{
    led_row   = 0;
    led_color = 0;

    led_phase = 0;
    ul_phase  = 0;
    regen_row = 0;
    regen_col = 0;

    // restore the last saved settings (defaults applied on a fresh flash or
    // whenever the stored record is missing / has a different size)
    apply_defaults();
    settings_load();
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

    indicators_pwm_enable();
}

void indicators_next_effect()
{
    // OFF -> FX_RADIAL -> ... -> FX_SOLID -> OFF -> ...
    if (++user_settings.led_effect > FX_OFF) {
        user_settings.led_effect = 0;
    }

    settings_save();
}

void indicators_prev_effect()
{
    // OFF <- FX_RADIAL <- ... <- FX_SOLID <- OFF <- ...
    if (user_settings.led_effect == 0) {
        user_settings.led_effect = FX_OFF;
    } else {
        user_settings.led_effect--;
    }

    settings_save();
}

void indicators_brightness_up()
{
    if (user_settings.led_brightness > (uint8_t)(255 - LED_BRIGHTNESS_STEP)) {
        user_settings.led_brightness = 255;
    } else {
        user_settings.led_brightness = (uint8_t)(user_settings.led_brightness + LED_BRIGHTNESS_STEP);
    }

    settings_save();
}

void indicators_brightness_down()
{
    if (user_settings.led_brightness < LED_BRIGHTNESS_STEP) {
        user_settings.led_brightness = 0;
    } else {
        user_settings.led_brightness = (uint8_t)(user_settings.led_brightness - LED_BRIGHTNESS_STEP);
    }

    settings_save();
}

void indicators_speed_up()
{
    if (user_settings.led_speed < LED_SPEED_MAX) {
        user_settings.led_speed++;
    }

    settings_save();
}

void indicators_speed_down()
{
    if (user_settings.led_speed > LED_SPEED_MIN) {
        user_settings.led_speed--;
    }

    settings_save();
}

// Underglow / "user-light" controls. Identical logic to the main backlight ones,
// but operating on the independent ul_* fields of user_settings.

void indicators_ul_next_effect()
{
    if (++user_settings.ul_effect > FX_OFF) {
        user_settings.ul_effect = 0;
    }
    settings_save();
}

void indicators_ul_prev_effect()
{
    if (user_settings.ul_effect == 0) {
        user_settings.ul_effect = FX_OFF;
    } else {
        user_settings.ul_effect--;
    }
    settings_save();
}

void indicators_ul_brightness_up()
{
    if (user_settings.ul_brightness > (uint8_t)(255 - LED_BRIGHTNESS_STEP)) {
        user_settings.ul_brightness = 255;
    } else {
        user_settings.ul_brightness = (uint8_t)(user_settings.ul_brightness + LED_BRIGHTNESS_STEP);
    }
    settings_save();
}

void indicators_ul_brightness_down()
{
    if (user_settings.ul_brightness < LED_BRIGHTNESS_STEP) {
        user_settings.ul_brightness = 0;
    } else {
        user_settings.ul_brightness = (uint8_t)(user_settings.ul_brightness - LED_BRIGHTNESS_STEP);
    }
    settings_save();
}

void indicators_ul_speed_up()
{
    if (user_settings.ul_speed < LED_SPEED_MAX) {
        user_settings.ul_speed++;
    }
    settings_save();
}

void indicators_ul_speed_down()
{
    if (user_settings.ul_speed > LED_SPEED_MIN) {
        user_settings.ul_speed--;
    }
    settings_save();
}

void indicators_pre_update()
{
    // set all rgb sinks to low (animation step will enable needed ones)
    P0 &= ~(RGB_R2R_P0_2 | RGB_R0B_P0_3 | RGB_R0R_P0_4);
    P1 &= ~(RGB_ULR_P1_1 | RGB_ULG_P1_2 | RGB_ULB_P1_3);
    P4 &= ~(RGB_R4B_P4_3 | RGB_R4R_P4_4 | RGB_R3R_P4_5 | RGB_R3B_P4_6);
    P5 &= ~(RGB_R2B_P5_7);
    P6 &= ~(RGB_R0G_P6_1 | RGB_R1G_P6_2 | RGB_R2G_P6_3 | RGB_R3G_P6_4 | RGB_R4G_P6_5 | RGB_R1B_P6_6 | RGB_R1R_P6_7);

    indicators_pwm_disable();
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
    if (led_row < LED_ROWS) {
        if (user_settings.led_effect < FX_OFF) {
            led_enable_sink();
            led_set_columns();
        }
    } else {
        led_enable_sink();
        led_set_columns();
    }

    if (++led_color >= 3) {
        led_color = 0;
        if (++led_row >= LED_SCAN_ROWS) {
            led_row = 0;
        }
    }

    return false;
}

void indicators_post_update()
{
    // clear pwm isr flag
    PWM00CON &= ~(1 << 5);

    indicators_pwm_enable();
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
            } else if (keyboard_state.rf_link == RF_MODE_2_4G) {
                led_ul_fb[0][regen_col] = 0;
                led_ul_fb[1][regen_col] = SCALE_UL_BRI(255);
                led_ul_fb[2][regen_col] = 0;
            } else {
                led_ul_fb[0][regen_col] = 0;
                led_ul_fb[1][regen_col] = 0;
                led_ul_fb[2][regen_col] = SCALE_UL_BRI(255);
#else
            } else {
                led_ul_fb[0][regen_col] = 0;
                led_ul_fb[1][regen_col] = 0;
                led_ul_fb[2][regen_col] = 0;
#endif
            }
        }
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
    // TODO: try abstracting individual banks away
    PWM00CON = (uint8_t)(PWM_MODE_ENABLE_BIT | PWM_INT_ENABLE_BIT | PWM_SS_BIT | PWM_CLK_DIV);
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

    PWM20CON = (uint8_t)(PWM_MODE_ENABLE_BIT | PWM_SS_BIT | PWM_CLK_DIV);
    // PWM24CON = PWM_SS_BIT;
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
