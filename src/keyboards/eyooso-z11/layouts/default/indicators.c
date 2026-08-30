#include "indicators.h"
#include "sh68f90a.h"
#include "kbdef.h"
#include "gpio.h"
#include "pwm.h"
#include "settings.h"
#include "led_effect.h"
#include "user_led.h"

#define LED_ROWS MATRIX_ROWS
#define LED_COLS MATRIX_COLS

#define LED_PHASE_SPEED 4

#define LED_DUTY(v) (uint16_t)(0x0400u - ((uint16_t)(v) << 2))

#define LED_ALL_ROWS (uint8_t)(LED_R0_P6_1 | LED_R1_P6_2 | LED_R2_P6_3 | LED_R3_P6_4 | LED_R4_P6_5)

#include LED_GEOMETRY_HEADER
_Static_assert(LED_GEOMETRY_ROWS == LED_ROWS && LED_GEOMETRY_COLS == LED_COLS, "generated LED geometry size does not match the key matrix");

static uint8_t led_fb[LED_ROWS][LED_COLS];

static uint8_t led_row;
static uint8_t led_phase;
static uint8_t regen_row;
static uint8_t regen_col;

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

    user_settings.led_effect = FX_RADIAL;
    settings_load();
    if (user_settings.led_effect > FX_OFF) {
        user_settings.led_effect = FX_RADIAL;
    }

    indicators_pwm_enable();
}

void indicators_next_effect()
{
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
    GPIO_HIGH(6, LED_ALL_ROWS);

    indicators_pwm_disable();
}

bool indicators_update_step(keyboard_state_t *keyboard, uint8_t current_step)
{
    current_step;

    LED_CAPS = !(keyboard->led_state & (1 << 1));

    if (user_settings.led_effect >= FX_OFF) {
        return false;
    }

    led_regen_one();

    led_enable_row();
    led_set_columns();

    if (++led_row >= LED_ROWS) {
        led_row = 0;
    }

    return false;
}

void indicators_post_update()
{
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
    PWM00CON = (uint8_t)(PWM_MODE_ENABLE | PWM_INT_ENABLE | PWM_SS | PWM_CLK_DIV_4);
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

    PWM20CON = (uint8_t)(PWM_MODE_ENABLE | PWM_SS | PWM_CLK_DIV_4);
    PWM24CON = PWM_SS;
    PWM25CON = PWM_SS;
}

void indicators_pwm_disable()
{
    // TODO: try abstracting individual banks away
    PWM00CON = PWM_CON_PARKED;
    PWM01CON = PWM_CON_OFF;
    PWM02CON = PWM_CON_OFF;
    PWM03CON = PWM_CON_OFF;
    PWM04CON = PWM_CON_OFF;
    PWM05CON = PWM_CON_OFF;

    PWM10CON = PWM_CON_PARKED;
    PWM11CON = PWM_CON_OFF;
    PWM12CON = PWM_CON_OFF;
    PWM13CON = PWM_CON_OFF;
    PWM14CON = PWM_CON_OFF;
    PWM15CON = PWM_CON_OFF;

    PWM20CON = PWM_CON_PARKED;
    PWM24CON = PWM_CON_OFF;
    PWM25CON = PWM_CON_OFF;
}
