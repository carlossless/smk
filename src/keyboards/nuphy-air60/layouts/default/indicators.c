#include "../../../../user/indicators.h"
#include "../../../../lib/sh68f90a/sh68f90a.h"
#include "../../../../lib/sh68f90a/pwm.h"
#include <stdlib.h>

void indicators_pre_update()
{
    // set all rgb sinks to low (animation step will enable needed ones)
    P0 &= ~(_P0_2 | _P0_3 | _P0_4);
    P1 &= ~(_P1_1 | _P1_2 | _P1_3);
    P4 &= ~(_P4_3 | _P4_4 | _P4_5 | _P4_6);
    P5 &= ~(_P5_7);
    P6 &= ~(_P6_1 | _P6_2 | _P6_3 | _P6_4 | _P6_5 | _P6_6 | _P6_7);

    pwm_disable();
}

bool indicators_update_step(keyboard_state_t *keyboard, uint8_t current_step)
{
    static uint16_t current_cycle = 0;

    if (current_step == 0) {
        if (current_cycle < 3072) {
            current_cycle++;
        } else {
            current_cycle = 0;
        }
    }

    uint16_t red_intensity = 0;
    uint16_t green_intensity = 0;
    uint16_t blue_intensity = 0;

    if (current_cycle < 1024) {
        blue_intensity = 1024 - (uint16_t)abs((int16_t)((current_cycle + 1024) % 2048) - 1024);
        red_intensity = 1024 - (uint16_t)abs((int16_t)((current_cycle) % 2048) - 1024);
    } else if (current_cycle >= 1024 && current_cycle < 2048) {
        red_intensity = 1024 - (uint16_t)abs((int16_t)((current_cycle) % 2048) - 1024);
        green_intensity = 1024 - (uint16_t)abs((int16_t)((current_cycle + 1024) % 2048) - 1024);
    } else if (current_cycle >= 2048) {
        green_intensity = 1024 - (uint16_t)abs((int16_t)((current_cycle + 1024) % 2048) - 1024);
        blue_intensity = 1024 - (uint16_t)abs((int16_t)((current_cycle) % 2048) - 1024);
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
            P0_4 = 1;
            P6_7 = 1;
            P0_2 = 1;
            P4_5 = 1;
            P4_4 = 1;
            P1_1 = 1;
            color_intensity = red_intensity;
            break;

        case 1: // green
            P6_1 = 1;
            P6_2 = 1;
            P6_3 = 1;
            P6_4 = 1;
            P6_5 = 1;
            P1_2 = 1;
            color_intensity = green_intensity;
            break;

        case 2: // blue
            P0_3 = 1;
            P6_6 = 1;
            P5_7 = 1;
            P4_6 = 1;
            P4_3 = 1;
            P1_3 = 1;
            color_intensity = blue_intensity;
            break;

        default:
            // unreachable
            color_intensity = 0;
            break;
    }

    // set pwm duty cycles to expected colors
    pwm_set_all_columns(color_intensity);

    return false;
}

void indicators_post_update()
{
    // clear pwm isr flag
    PWM00CON &= ~(1 << 5);

    pwm_enable();
}
