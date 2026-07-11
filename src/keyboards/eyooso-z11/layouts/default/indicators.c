#include "indicators.h"
#include "sh68f90a.h"
#include "kbdef.h"
#include "pwm.h"
#include "settings.h"
#include "led_effect.h"
#include "user_led.h"

// TODO: move these defines out
#define PWM_CLK_DIV         0b010 // PWM_CLK = SYS_CLK / 4
#define PWM_SS_BIT          (1 << 3)
#define PWM_MOD_BIT         (1 << 4)
#define PWM_INT_ENABLE_BIT  (1 << 6)
#define PWM_MODE_ENABLE_BIT (1 << 7)

#define LED_ROWS MATRIX_ROWS
#define LED_COLS MATRIX_COLS

// Phase increment per full framebuffer sweep. Larger = faster.
#define LED_PHASE_SPEED 4

// 8-bit framebuffer brightness -> 10-bit column PWM duty (0 = full on, 0x400 = off)
#define LED_DUTY(v) (uint16_t)(0x0400u - ((uint16_t)(v) << 2))

#define LED_ALL_ROWS (uint8_t)(LED_R0_P6_1 | LED_R1_P6_2 | LED_R2_P6_3 | LED_R3_P6_4 | LED_R4_P6_5)

#include LED_GEOMETRY_HEADER
_Static_assert(LED_GEOMETRY_ROWS == LED_ROWS && LED_GEOMETRY_COLS == LED_COLS, "generated LED geometry size does not match the key matrix");

static __xdata uint8_t led_fb[LED_ROWS][LED_COLS];

static __xdata uint8_t led_row;
static __xdata uint8_t led_phase;
static __xdata uint8_t regen_row;
static __xdata uint8_t regen_col;

void        indicators_pwm_enable();
void        indicators_pwm_disable();
static void led_regen_one();
static void led_enable_row();
static void led_set_columns();

void indicators_start()
{
    led_row   = 0;
    led_phase = 0;
    regen_row = 0;
    regen_col = 0;

    // restore the last saved effect (defaults to radial on a fresh flash)
    user_settings.led_effect = FX_RADIAL;
    settings_load();
    if (user_settings.led_effect > FX_OFF) {
        user_settings.led_effect = FX_RADIAL;
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

void indicators_factory_reset()
{
    user_settings.led_effect = FX_RADIAL;
    settings_save();
}

void indicators_pre_update()
{
    // disable every LED row (active-low: high = off); the step re-enables one
    P6 |= LED_ALL_ROWS;

    indicators_pwm_disable();
}

bool indicators_update_step(keyboard_state_t *keyboard, uint8_t current_step) __reentrant
{
    current_step;

    // caps-lock indicator works regardless of backlight state
    LED_CAPS = !(keyboard->led_state & (1 << 1));

    if (user_settings.led_effect >= FX_OFF) {
        // backlight off: pre_update already disabled every row, so nothing lights
        return false;
    }

    // regenerate one LED of the framebuffer per ISR, spreading the work out
    led_regen_one();

    // light one row this substep, with per-column brightness from the framebuffer
    led_enable_row();
    led_set_columns();

    if (++led_row >= LED_ROWS) {
        led_row = 0;
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
    uint8_t v;
    if (led_effect_mono((led_effect_t)user_settings.led_effect, regen_row, regen_col, led_phase, &v)) {
        led_fb[regen_row][regen_col] = v;
    }

    if (++regen_col >= LED_COLS) {
        regen_col = 0;
        if (++regen_row >= LED_ROWS) {
            regen_row = 0;
            led_phase = (uint8_t)(led_phase + LED_PHASE_SPEED);
        }
    }
}

static void led_enable_row()
{
    switch (led_row) {
        case 0:
            LED_R0 = 0;
            break;
        case 1:
            LED_R1 = 0;
            break;
        case 2:
            LED_R2 = 0;
            break;
        case 3:
            LED_R3 = 0;
            break;
        case 4:
            LED_R4 = 0;
            break;
    }
}

static void led_set_columns()
{
    __xdata uint8_t *fb = led_fb[led_row];

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
    PWM24CON = PWM_SS_BIT;
    PWM25CON = PWM_SS_BIT;
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
    PWM24CON = 0;
    PWM25CON = 0;
}
