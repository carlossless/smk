#include "matrix.h"
#include "sh68f90a.h"
#include "pwm.h"
#include "delay.h"
#include <stdlib.h>
#include <math.h>

#define C0 P5_0
#define C1 P5_1
#define C2 P5_2
#define C3 P3_5
#define C4 P3_4
#define C5 P3_3
#define C6 P3_2
#define C7 P3_1
#define C8 P3_0
#define C9 P2_5
#define C10 P2_4
#define C11 P2_3
#define C12 P2_2
#define C13 P2_1
#define C14 P2_0
#define C15 P1_5

volatile uint8_t key_state[MATRIX_COLS];
volatile __xdata uint8_t prev_key_state[MATRIX_COLS];
volatile uint8_t current_step;
volatile __bit _int_key_state_changed;
volatile __bit key_state_changed;

volatile uint16_t current_cycle;

void matrix_init()
{
    current_step = 0;
    _int_key_state_changed = 0;
    key_state_changed = 0;

    for (int i = 0; i < MATRIX_COLS; i++) {
        key_state[i] = 0;
        prev_key_state[i] = 0;
    }

    current_cycle = 1;
}

inline void matrix_scan()
{
    // set all rgb sinks to low
    P0 &= ~((1 << 2) | (1 << 3) | (1 << 4));
    P1 &= ~((1 << 1) | (1 << 2) | (1 << 3));
    P4 &= ~((1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));
    P5 &= ~((1 << 7));
    P6 &= ~((1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7));

    pwm_disable();

    // ignore until key state has been read
    if (!key_state_changed) {
        // set all columns to high
        P1 |= (uint8_t)((1 << 5));
        P2 |= (uint8_t)((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));
        P3 |= (uint8_t)((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));
        P5 |= (uint8_t)((1 << 0) | (1 << 1) | (1 << 2));

        // set current (!) column to low
        switch(current_step) {
        case 0:
            C0 = 0;
            break;
        case 1:
            C1 = 0;
            break;
        case 2:
            C2 = 0;
            break;
        case 3:
            C3 = 0;
            break;
        case 4:
            C4 = 0;
            break;
        case 5:
            C5 = 0;
            break;
        case 6:
            C6 = 0;
            break;
        case 7:
            C7 = 0;
            break;
        case 8:
            C8 = 0;
            break;
        case 9:
            C9 = 0;
            break;
        case 10:
            C10 = 0;
            break;
        case 11:
            C11 = 0;
            break;
        case 12:
            C12 = 0;
            break;
        case 13:
            C13 = 0;
            break;
        case 14:
            C14 = 0;
            break;
        case 15:
            C15 = 0;
            break;
        }

        // P7_1 - R0
        // P7_2 - R1
        // P7_3 - R2
        // P5_3 - R3
        // P5_4 - R4

        // check rows for pressed keys
        uint8_t initial_row_state = (((P7 >> 1) & 0x07) | (P5 & 0x18)) | 0xe0;

        // set all columns down to low
        P1 &= (uint8_t)~((1 << 5));
        P2 &= (uint8_t)~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));
        P3 &= (uint8_t)~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));
        P5 &= (uint8_t)~((1 << 0) | (1 << 1) | (1 << 2));

        key_state[current_step] = ~initial_row_state;
        if (key_state[current_step] != prev_key_state[current_step]) {
            _int_key_state_changed = 1;
        }
        prev_key_state[current_step] = key_state[current_step];
    }

    // rgb led matrix animation
    uint16_t red_intensity = 0;
    uint16_t green_intensity = 0;
    uint16_t blue_intensity = 0;

    if (current_cycle >= 0 && current_cycle < 1024) {
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
    }

    // set pwm duty cycles to expected colors
    pwm_set_all_columns(color_intensity);

    // move step
    if (current_step < MATRIX_COLS-1) {
        current_step++;
    } else {
        current_step = 0;
        if (_int_key_state_changed) {
            key_state_changed = 1;
            _int_key_state_changed = 0;
        }
        if (current_cycle < 3072) {
            current_cycle++;
        } else {
            current_cycle = 0;
        }
    }

    // clear pwm isr flag
    PWM00CON &= ~(1 << 5);

    pwm_enable();
}
