#include "indicators.h"
#include "sh68f881.h"
#include "kbdef.h"
#include "settings.h"
#include "led_effect.h"
#include "user_led.h"

#define LED_ROWS MATRIX_ROWS
#define LED_COLS MATRIX_COLS

// animation step per frame, indexed by user_settings.led_speed.
static const __code uint8_t led_speeds[] = {1, 2, 4, 8};
#define LED_SPEED_LEVELS (sizeof(led_speeds))

// no PWM to turn down, so brightness dithers whole frames: an LED lights on `brightness + 1` of every LED_BRIGHTNESS_LEVELS.
static uint8_t led_frame;

// the columns are plain outputs, not PWM channels, so a subframe can only light an LED or not: anything at or above this shows.
#define LED_ON_THRESHOLD 0x80u

#include LED_GEOMETRY_HEADER
_Static_assert(LED_GEOMETRY_ROWS == LED_ROWS && LED_GEOMETRY_COLS == LED_COLS, "generated LED geometry size does not match the key matrix");
static __xdata uint8_t led_fb[LED_ROWS][LED_COLS];

static uint8_t led_col;
static uint8_t led_phase;
static uint8_t regen_row;
static uint8_t regen_col;

// one column low with up to six anodes high; both pages are forced because on page 1 the P1 anodes are P6.
static void led_blank(void)
{
    uint8_t saved_page = INSCON;

    sfr_page_0();
    P1 &= (uint8_t)~KB_ANODE_P1_MASK;

    sfr_page_1();
    P5 &= (uint8_t)~KB_ANODE_P5_MASK;
    P6 = _P6_ALL;
    P7 = _P7_ALL;
    P8 = _P8_ALL;

    INSCON = saved_page;
}

static void led_drive(uint8_t col, uint8_t anodes)
{
    uint8_t saved_page = INSCON;
    uint8_t mask       = kb_col_masks[col];

    sfr_page_1();
    P6 = (col < KB_C_P7_FIRST) ? (uint8_t)~mask : _P6_ALL;
    P7 = (col >= KB_C_P7_FIRST && col < KB_C_P8_FIRST) ? (uint8_t)~mask : _P7_ALL;
    P8 = (col >= KB_C_P8_FIRST) ? (uint8_t)~mask : _P8_ALL;
    P5 = (uint8_t)((P5 & ~KB_ANODE_P5_MASK) | ((anodes >> 3) & KB_ANODE_P5_MASK));

    sfr_page_0();
    P1 = (uint8_t)((P1 & ~KB_ANODE_P1_MASK) | (anodes & KB_ANODE_P1_MASK));

    INSCON = saved_page;
}

// one effect evaluation per subframe: six, one per row, does not fit and starves the USB interrupt.
static void led_regen_one(void)
{
    uint8_t v;
    if (led_effect_mono((led_effect_t)user_settings.led_effect, regen_row, regen_col, led_phase, &v)) {
        led_fb[regen_row][regen_col] = v;
    }

    if (++regen_col >= LED_COLS) {
        regen_col = 0;
        if (++regen_row >= LED_ROWS) {
            regen_row = 0;
            led_phase = (uint8_t)(led_phase + led_speeds[user_settings.led_speed]);
        }
    }
}

void indicators_start(void)
{
    led_col   = 0;
    led_phase = 0;
    regen_row = 0;
    regen_col = 0;
    led_frame = 0;

    user_settings.led_effect = FX_RADIAL;
    settings_load();
    if (user_settings.led_effect > FX_OFF) {
        user_settings.led_effect = FX_RADIAL;
    }
    if (user_settings.led_brightness >= LED_BRIGHTNESS_LEVELS) {
        user_settings.led_brightness = LED_BRIGHTNESS_LEVELS - 1;
    }
    if (user_settings.led_speed >= LED_SPEED_LEVELS) {
        user_settings.led_speed = 2;
    }
}

void indicators_next_effect(void)
{
    if (++user_settings.led_effect > FX_OFF) {
        user_settings.led_effect = 0;
    }

    settings_save();
}

void indicators_set_effect(uint8_t fx)
{
    if (fx > FX_OFF) {
        return;
    }

    user_settings.led_effect = fx;
    settings_save();
}

void indicators_step_brightness(bool up)
{
    if (up) {
        if (user_settings.led_brightness + 1u >= LED_BRIGHTNESS_LEVELS) {
            return;
        }
        user_settings.led_brightness++;
    } else {
        if (user_settings.led_brightness == 0) {
            return;
        }
        user_settings.led_brightness--;
    }

    settings_save();
}

void indicators_step_speed(bool up)
{
    if (up) {
        if (user_settings.led_speed + 1u >= LED_SPEED_LEVELS) {
            return;
        }
        user_settings.led_speed++;
    } else {
        if (user_settings.led_speed == 0) {
            return;
        }
        user_settings.led_speed--;
    }

    settings_save();
}

void indicators_factory_reset(void)
{
    user_settings.led_effect     = FX_RADIAL;
    user_settings.led_brightness = LED_BRIGHTNESS_LEVELS - 1;
    user_settings.led_speed      = 2;
    settings_save();
}

void indicators_pre_update(void)
{
    led_blank();
}

bool indicators_update_step(keyboard_state_t *keyboard, uint8_t current_step)
{
    (void)current_step;
    (void)keyboard;

    if (user_settings.led_effect >= FX_OFF) {
        return false;
    }

    uint8_t anodes = 0;

    // dark dither frames still advance the column and the animation, or lower brightness would also slow the effect.
    if (led_frame <= user_settings.led_brightness) {
        for (uint8_t row = 0; row < LED_ROWS; row++) {
            if (led_fb[row][led_col] >= LED_ON_THRESHOLD) {
                anodes |= (uint8_t)(1u << row);
            }
        }
    }

    led_regen_one();

    led_drive(led_col, anodes);

    bool wrapped = false;
    if (++led_col >= LED_COLS) {
        led_col = 0;
        wrapped = true;
        if (++led_frame >= LED_BRIGHTNESS_LEVELS) {
            led_frame = 0;
        }
    }

    return wrapped;
}

void indicators_post_update(void) {}

void indicators_pwm_disable(void)
{
    led_blank();
}

void indicators_pwm_enable(void) {}
