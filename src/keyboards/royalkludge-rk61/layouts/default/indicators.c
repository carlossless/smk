#include "indicators.h"
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
    indicators_pwm_set_all_columns(0);

    /*
    static uint16_t current_cycle = 0;

    if (current_step == 0) {
        if (current_cycle < 3072) {
            current_cycle++;
        } else {
            current_cycle = 0;
        }
    }

    uint16_t red_intensity   = 0;
    uint16_t green_intensity = 0;
    uint16_t blue_intensity  = 0;

    if (current_cycle < 1024) {
        blue_intensity = 1024 - (uint16_t)abs((int16_t)((current_cycle + 1024) % 2048) - 1024);
        red_intensity  = 1024 - (uint16_t)abs((int16_t)((current_cycle) % 2048) - 1024);
    } else if (current_cycle < 2048) {
        red_intensity   = 1024 - (uint16_t)abs((int16_t)((current_cycle) % 2048) - 1024);
        green_intensity = 1024 - (uint16_t)abs((int16_t)((current_cycle + 1024) % 2048) - 1024);
    } else {
        green_intensity = 1024 - (uint16_t)abs((int16_t)((current_cycle + 1024) % 2048) - 1024);
        blue_intensity  = 1024 - (uint16_t)abs((int16_t)((current_cycle) % 2048) - 1024);
    }

    uint16_t color_intensity;

    if (keyboard->led_state & (1 << 0)) { // num_lock
        red_intensity = 1024;
    }

    if (keyboard->led_state & (1 << 1)) { // caps_lock
        green_intensity = 1024;
    }

    if (keyboard->led_state & (1 << 2)) { // scroll_lock
        blue_intensity = 1024;
    }

    switch (current_step % 3) {
        case 0: // red
            RGB_R0R = 1;
            RGB_R1R = 1;
            RGB_R2R = 1;
            RGB_R3R = 1;
            RGB_R4R = 1;
            RGB_ULR = 1;

            color_intensity = red_intensity;
            break;

        case 1: // green
            RGB_R0G = 1;
            RGB_R1G = 1;
            RGB_R2G = 1;
            RGB_R3G = 1;
            RGB_R4G = 1;
            RGB_ULG = 1;

            color_intensity = green_intensity;
            break;

        case 2: // blue
            RGB_R0B = 1;
            RGB_R1B = 1;
            RGB_R2B = 1;
            RGB_R3B = 1;
            RGB_R4B = 1;
            RGB_ULB = 1;

            color_intensity = blue_intensity;
            break;

        default:
            // unreachable
            color_intensity = 0;
            break;
    }

    // set pwm duty cycles to expected colors
    indicators_pwm_set_all_columns(color_intensity);
    */

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
    SET_PWM_DUTY_2(LED_PWM_C14, adjusted);
    SET_PWM_DUTY_2(LED_PWM_C15, adjusted);
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
