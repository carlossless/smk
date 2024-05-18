#include "indicators.h"
#include "sh68f90a.h"
#include "pwm.h"
#include "kbdef.h"
#include <stdlib.h>

void indicators_pre_update()
{
    // set all rgb sinks to low (animation step will enable needed ones)
    P6 &= ~(LED_R0_P6_1 | LED_R1_P6_2 | LED_R2_P6_3 | LED_R3_P6_4 | LED_R4_P6_5);

    pwm_disable();
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

    uint16_t intensity   = 0;

    intensity = 1024 - (uint16_t)abs((int16_t)((current_cycle + 1024) % 2048) - 1024);

    if (keyboard->led_state & (1 << 0)) { // num_lock
        intensity = 1024;
    }

    if (keyboard->led_state & (1 << 1)) { // caps_lock
        intensity = 1024;
    }

    if (keyboard->led_state & (1 << 2)) { // scroll_lock
        intensity = 1024;
    }

    // probably not necessary
    // LED_R0 = 1;
    // LED_R1 = 1;
    // LED_R2 = 1;
    // LED_R3 = 1;
    // LED_R4 = 1;

    // set pwm duty cycles to expected colors
    pwm_set_all_columns(intensity);

    return false;
}

void indicators_post_update()
{
    // clear pwm isr flag
    PWM00CON &= ~(1 << 5);

    pwm_enable();
}
