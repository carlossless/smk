#include "indicators.h"
#include "kbdef.h"
#include "pwm.h"
#include "settings.h"
#include "tick.h"
#include "keyboard.h"
#include "led_effect.h"
#include "user_led.h"
#include <string.h>
#ifdef RF_ENABLED
#    include "rf_controller.h"
#endif

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

#define UL_STATUS_COLS (LED_COLS / 2)

#define BAT_FLASH_SWEEPS 200

// Brightness step for one Fn+Up / Fn+Down press (out of 256 levels).
#define LED_BRIGHTNESS_DEFAULT 255
#define LED_BRIGHTNESS_STEP    32

// 8-bit framebuffer brightness -> column PWM DUTY2 register. DUTY1=0 transitions
// the output LOW at the start of the period, then DUTY2 transitions it back HIGH;
// the LED conducts while LOW, so LED-on-time = DUTY2 cycles. A direct fb → DUTY2
// mapping gives fb=0 → off, fb=255 → ~99.6% on. Do NOT invert: mapping fb=0 to
// DUTY2=0xFF makes "off" subframes glow at full brightness.
#define LED_DUTY(v) (uint16_t)(v)

// Scale an 8-bit channel by the current brightness (main and underglow are independent).
#define SCALE_BRI(v)    (uint8_t)(((uint16_t)(uint8_t)(v) * user_settings.led_brightness) >> 8)
#define SCALE_UL_BRI(v) (uint8_t)(((uint16_t)(uint8_t)(v) * user_settings.ul_brightness) >> 8)

#include LED_GEOMETRY_HEADER
_Static_assert(LED_GEOMETRY_ROWS == LED_ROWS && LED_GEOMETRY_COLS == LED_COLS, "generated LED geometry size does not match the key matrix");

static __xdata uint8_t led_fb[LED_ROWS][3][LED_COLS];

// Separate framebuffer for the underglow ("user") LEDs, since they animate
// independently of the main backlight.
static __xdata uint8_t led_ul_fb[3][LED_COLS];

static __xdata uint8_t led_row;
static __xdata uint8_t led_color;

static __xdata uint8_t led_phase;
static __xdata uint8_t ul_phase;
static __xdata uint8_t regen_row;
static __xdata uint8_t regen_col;

static __xdata uint8_t anim_ctr;

static volatile __xdata bool render_dirty;

static __xdata uint8_t battery_flash_sweeps;

static __xdata uint8_t status_pulse_counter;

static __code const uint8_t breath_lut[32] = {0, 13, 26, 39, 51, 64, 76, 88, 100, 112, 124, 135, 146, 156, 166, 176, 185, 194, 202, 210, 217, 223, 229, 234, 239, 244, 247, 251, 253, 254, 255, 255};

void        indicators_pwm_enable();
void        indicators_pwm_disable();
static void led_regen_one();
static void led_enable_sink();
static bool led_set_columns();

// Sets every field of user_settings to its factory default value.
void indicators_apply_defaults()
{
    user_settings.led_effect           = FX_RADIAL;
    user_settings.led_brightness       = LED_BRIGHTNESS_DEFAULT;
    user_settings.led_speed            = LED_SPEED_DEFAULT;
    user_settings.ul_effect            = FX_RADIAL;
    user_settings.ul_brightness        = LED_BRIGHTNESS_DEFAULT;
    user_settings.ul_speed             = LED_UL_SPEED_DEFAULT;
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

void indicators_factory_reset()
{
    indicators_apply_defaults();
    settings_mark_dirty();
}

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

void indicators_init()
{
    memset(led_fb, 0, sizeof(led_fb));
    memset(led_ul_fb, 0, sizeof(led_ul_fb));
}

void indicators_start()
{
    led_row      = 0;
    led_color    = 0;
    anim_ctr     = 0;
    render_dirty = true; // paint an initial frame

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
    P0 &= ~(RGB_R2R_P0_2 | RGB_R0B_P0_3 | RGB_R0R_P0_4);
    P1 &= ~(RGB_ULR_P1_1 | RGB_ULG_P1_2 | RGB_ULB_P1_3);
    P4 &= ~(RGB_R4B_P4_3 | RGB_R4R_P4_4 | RGB_R3R_P4_5 | RGB_R3B_P4_6);
    P5 &= ~(RGB_R2B_P5_7);
    P6 &= ~(RGB_R0G_P6_1 | RGB_R1G_P6_2 | RGB_R2G_P6_3 | RGB_R3G_P6_4 | RGB_R4G_P6_5 | RGB_R1B_P6_6 | RGB_R1R_P6_7);

}

void indicators_render()
{
    if (!render_dirty) {
        return;
    }
    render_dirty = false; // cleared first: a phase bump mid-render re-arms it

    for (uint8_t i = 0; i < (uint8_t)(LED_SCAN_ROWS * LED_COLS); i++) {
        led_regen_one();
    }
}

bool indicators_update_step(keyboard_state_t *keyboard, uint8_t current_step)
{
    keyboard;
    current_step;

    if (++anim_ctr >= (uint8_t)(LED_SCAN_ROWS * LED_COLS)) {
        anim_ctr = 0;
        ul_phase = (uint8_t)(ul_phase + user_settings.ul_speed);
        if (battery_flash_sweeps) {
            battery_flash_sweeps--;
        }
        status_pulse_counter++;
        render_dirty = true; // UL/status advanced — repaint the frame
    } else if (anim_ctr == (uint8_t)(LED_ROWS * LED_COLS)) {
        led_phase    = (uint8_t)(led_phase + user_settings.led_speed);
        render_dirty = true; // main effect advanced — repaint the frame
    }

    indicators_pwm_disable();

    bool lit = false;
    if (led_row < LED_ROWS) {
        if (user_settings.led_effect < FX_OFF) {
            lit = led_set_columns(); // load 16 duties while parked
        }
    } else {
        lit = led_set_columns();
    }

    if (lit) {
        led_enable_sink();       // raise this subframe's sink while parked
        indicators_pwm_enable(); // re-enable last — the selected row lights now
    }

    bool frame_wrapped = false;
    if (++led_color >= 3) {
        led_color = 0;
        if (++led_row >= LED_SCAN_ROWS) {
            led_row       = 0;
            frame_wrapped = true;
        }
    }

    return frame_wrapped;
}

void indicators_post_update()
{
    PWM00CON &= ~(1 << 5);
}

static void led_regen_one()
{
    if (regen_row < LED_ROWS) {
        // -------- MAIN backlight cell --------
        uint8_t rgb[3];
        if (led_effect_rgb((led_effect_t)user_settings.led_effect, regen_row, regen_col, led_phase, user_settings.led_brightness, rgb)) {
            led_fb[regen_row][0][regen_col] = rgb[0];
            led_fb[regen_row][1][regen_col] = rgb[1];
            led_fb[regen_row][2][regen_col] = rgb[2];
        }
    } else {
        uint8_t r, g, b;
        if (regen_col < UL_STATUS_COLS) {
            if (keyboard_state.led_state & (1 << 1)) {
                r = 50;
                g = 120;
                b = 50;
            } else if (CONN_MODE_SWITCH) {
                r = 255;
                g = 180;
                b = 0;
            } else {
#ifdef RF_ENABLED
                r = 0;
                g = 0;
                b = 0;
                if (keyboard_state.rf_link == RF_MODE_2_4G) {
                    g = 255;
                } else {
                    b = 255;
                }

                uint8_t status_scale = 255;
                if (!keyboard_state.paired) {
                    status_scale = (status_pulse_counter & 0x04) ? 255 : 0;
                } else if (!keyboard_state.connected) {
                    uint8_t p    = status_pulse_counter & 0x3F;          // 0..63
                    uint8_t idx  = (p & 0x20) ? (uint8_t)(0x3F - p) : p; // 0..31, mirrored
                    status_scale = breath_lut[idx];
                }
                r = (uint8_t)(((uint16_t)r * status_scale) >> 8);
                g = (uint8_t)(((uint16_t)g * status_scale) >> 8);
                b = (uint8_t)(((uint16_t)b * status_scale) >> 8);
#else
                r = 0;
                g = 0;
                b = 0;
#endif
            }
        }
#ifdef RF_ENABLED
        else if (!CONN_MODE_SWITCH && (user_settings.battery_indicator_on || battery_flash_sweeps)) {
            if (keyboard_state.low_power || keyboard_state.battery_level <= 1) {
                r = 255;
                g = 0;
                b = 0;
            } else if (keyboard_state.battery_level >= 6) {
                r = 0;
                g = 255;
                b = 0;
            } else {
                r = 255;
                g = 180;
                b = 0;
            }
        }
#endif
        else {
            // Right side, no battery overlay: the underglow effect itself.
            if (user_settings.ul_effect == FX_SOLID) {
                r = 255;
                g = 255;
                b = 255;
            } else if (user_settings.ul_effect < FX_OFF) {
                uint8_t h;
                switch (user_settings.ul_effect) {
                    case FX_HORIZONTAL:
                        h = (uint8_t)(user_led_axis_x(regen_col) + ul_phase);
                        break;
                    case FX_VERTICAL:
                        // UL is one strip, so vertical = whole strip cycling together
                        h = ul_phase;
                        break;
                    case FX_RADIAL:
                    default: {
                        uint8_t d = (regen_col < (LED_COLS / 2)) ? (uint8_t)((LED_COLS / 2 - 1) - regen_col) : (uint8_t)(regen_col - LED_COLS / 2);
                        h         = (uint8_t)((uint8_t)(d * 36) + ul_phase);
                        break;
                    }
                }
                uint8_t rgb[3];
                led_color_wheel(h, rgb);
                r = rgb[0];
                g = rgb[1];
                b = rgb[2];
            } else {
                r = 0;
                g = 0;
                b = 0; // UL effect off
            }
        }

        led_ul_fb[0][regen_col] = SCALE_UL_BRI(r);
        led_ul_fb[1][regen_col] = SCALE_UL_BRI(g);
        led_ul_fb[2][regen_col] = SCALE_UL_BRI(b);
    }

    if (++regen_col >= LED_COLS) {
        regen_col = 0;
        if (++regen_row >= LED_SCAN_ROWS) {
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

static bool led_set_columns()
{
    // underglow uses its own framebuffer; key rows use the main one
    __xdata uint8_t *fb = (led_row == LED_UL_ROW) ? led_ul_fb[led_color] : led_fb[led_row][led_color];

    uint8_t any = 0;
    SET_PWM_DUTY_2(LED_PWM_C0, LED_DUTY(fb[0]));
    any = (uint8_t)(any | fb[0]);
    SET_PWM_DUTY_2(LED_PWM_C1, LED_DUTY(fb[1]));
    any = (uint8_t)(any | fb[1]);
    SET_PWM_DUTY_2(LED_PWM_C2, LED_DUTY(fb[2]));
    any = (uint8_t)(any | fb[2]);
    SET_PWM_DUTY_2(LED_PWM_C3, LED_DUTY(fb[3]));
    any = (uint8_t)(any | fb[3]);
    SET_PWM_DUTY_2(LED_PWM_C4, LED_DUTY(fb[4]));
    any = (uint8_t)(any | fb[4]);
    SET_PWM_DUTY_2(LED_PWM_C5, LED_DUTY(fb[5]));
    any = (uint8_t)(any | fb[5]);
    SET_PWM_DUTY_2(LED_PWM_C6, LED_DUTY(fb[6]));
    any = (uint8_t)(any | fb[6]);
    SET_PWM_DUTY_2(LED_PWM_C7, LED_DUTY(fb[7]));
    any = (uint8_t)(any | fb[7]);
    SET_PWM_DUTY_2(LED_PWM_C8, LED_DUTY(fb[8]));
    any = (uint8_t)(any | fb[8]);
    SET_PWM_DUTY_2(LED_PWM_C9, LED_DUTY(fb[9]));
    any = (uint8_t)(any | fb[9]);
    SET_PWM_DUTY_2(LED_PWM_C10, LED_DUTY(fb[10]));
    any = (uint8_t)(any | fb[10]);
    SET_PWM_DUTY_2(LED_PWM_C11, LED_DUTY(fb[11]));
    any = (uint8_t)(any | fb[11]);
    SET_PWM_DUTY_2(LED_PWM_C12, LED_DUTY(fb[12]));
    any = (uint8_t)(any | fb[12]);
    SET_PWM_DUTY_2(LED_PWM_C13, LED_DUTY(fb[13]));
    any = (uint8_t)(any | fb[13]);
    SET_PWM_DUTY_2(LED_PWM_C14, LED_DUTY(fb[14]));
    any = (uint8_t)(any | fb[14]);
    SET_PWM_DUTY_2(LED_PWM_C15, LED_DUTY(fb[15]));
    any = (uint8_t)(any | fb[15]);
    return any != 0;
}

void indicators_pwm_enable()
{
    PWM00CON = (uint8_t)(PWM_MODE_ENABLE | PWM_SS | PWM_CLK_DIV_4);
    PWM01CON = PWM_SS;
    PWM02CON = PWM_SS;
    PWM03CON = PWM_SS;
    PWM04CON = PWM_SS;
    PWM05CON = PWM_SS;

    PWM10CON = (uint8_t)(PWM_MODE_ENABLE | PWM_SS | PWM_CLK_DIV_4);
    PWM11CON = PWM_SS;
    PWM12CON = PWM_SS;
    PWM13CON = PWM_SS;
    PWM14CON = PWM_SS;
    PWM15CON = PWM_SS;

    PWM20CON = (uint8_t)(PWM_MODE_ENABLE | PWM_CLK_DIV_4);
    PWM21CON = 0;
    PWM22CON = 0;
    PWM23CON = 0;
    PWM24CON = PWM_SS;
    PWM25CON = PWM_SS;

    PWM40CON = (uint8_t)(PWM_MODE_ENABLE | PWM_SS | PWM_CLK_DIV_4);
    PWM41CON = PWM_SS;
    PWM42CON = PWM_SS;
}

void settings_save_pre(void)
{
    tick_pause();
    indicators_pwm_disable();
}

void settings_save_post(void)
{
    indicators_pwm_enable();
    tick_resume();
}

void indicators_pwm_disable()
{
    // Park every channel, not just the bank masters: that is what keeps the
    // per-subframe re-enable glitch-free. Used both for the matrix-scan hand-off
    // (the columns revert to GPIO) and for the park in indicators_update_step.
    PWM00CON = PWM_CON_PARKED;
    PWM01CON = PWM_CON_PARKED;
    PWM02CON = PWM_CON_PARKED;
    PWM03CON = PWM_CON_PARKED;
    PWM04CON = PWM_CON_PARKED;
    PWM05CON = PWM_CON_PARKED;
    PWM10CON = PWM_CON_PARKED;
    PWM11CON = PWM_CON_PARKED;
    PWM12CON = PWM_CON_PARKED;
    PWM13CON = PWM_CON_PARKED;
    PWM14CON = PWM_CON_PARKED;
    PWM15CON = PWM_CON_PARKED;
    PWM20CON = PWM_CON_PARKED;
    PWM24CON = PWM_CON_PARKED;
    PWM25CON = PWM_CON_PARKED;
    PWM40CON = PWM_CON_PARKED;
    PWM41CON = PWM_CON_PARKED;
    PWM42CON = PWM_CON_PARKED;
}
