#include "indicators.h"
#include "sh68f90a.h"
#include "kbdef.h"
#include "pwm.h"
#include <stdlib.h>

// TODO: move these defines out
#define PWM_CLK_DIV         0b010 // PWM_CLK = SYS_CLK / 4
#define PWM_SS_BIT          (1 << 3)
#define PWM_MOD_BIT         (1 << 4)
#define PWM_INT_ENABLE_BIT  (1 << 6)
#define PWM_MODE_ENABLE_BIT (1 << 7)

void indicators_pwm_set_all_columns(uint16_t intensity);
void indicators_pwm_enable();
void indicators_pwm_disable();

void indicators_start()
{
    indicators_pwm_enable();
}

void indicators_pre_update()
{
    // set all rgb sinks to low (animation step will enable needed ones)
    P6 &= ~(LED_R0_P6_1 | LED_R1_P6_2 | LED_R2_P6_3 | LED_R3_P6_4 | LED_R4_P6_5);

    indicators_pwm_disable();
}

bool indicators_update_step(keyboard_state_t *keyboard, uint8_t current_step)
{
    static uint16_t current_cycle = 0;

    if (current_step == 0) {
        if (current_cycle < 2048) {
            current_cycle++;
        } else {
            current_cycle = 0;
        }
    }

    uint16_t intensity = 0;

    intensity = 1024 - (uint16_t)abs((int16_t)((current_cycle + 1024) % 2048) - 1024);

    LED_CAPS = !(keyboard->led_state & (1 << 1));

    // set pwm duty cycles to expected colors
    indicators_pwm_set_all_columns(intensity);

    return false;
}

void indicators_post_update()
{
    // clear pwm isr flag
    PWM00CON &= ~(1 << 5);

    indicators_pwm_enable();
}

void indicators_pwm_set_all_columns(uint16_t intensity)
{
    uint16_t adjusted = 0x0400 - intensity;

    SET_PWM_DUTY_2(LED_PWM_C0, adjusted);
    SET_PWM_DUTY_2(LED_PWM_C1, adjusted);
    SET_PWM_DUTY_2(LED_PWM_C2, adjusted);
    SET_PWM_DUTY_2(LED_PWM_C3, adjusted);
    SET_PWM_DUTY_2(LED_PWM_C4, adjusted);
    SET_PWM_DUTY_2(LED_PWM_C5, adjusted);
    SET_PWM_DUTY_2(LED_PWM_C6, adjusted);
    SET_PWM_DUTY_2(LED_PWM_C7, adjusted);
    SET_PWM_DUTY_2(LED_PWM_C8, adjusted);
    SET_PWM_DUTY_2(LED_PWM_C9, adjusted);
    SET_PWM_DUTY_2(LED_PWM_C10, adjusted);
    SET_PWM_DUTY_2(LED_PWM_C11, adjusted);
    SET_PWM_DUTY_2(LED_PWM_C12, adjusted);
    SET_PWM_DUTY_2(LED_PWM_C13, adjusted);
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
