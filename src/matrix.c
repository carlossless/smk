#include "matrix.h"
#include "sh68f90a.h"
#include "pwm.h"
#include "delay.h"
#include "layout.h"
#include "report.h"
#include "debug.h"
#include <stdlib.h>
#include <stdbool.h>

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

void animation_step(uint8_t current_step);

typedef uint8_t matrix_col_t;

volatile uint8_t current_step;

volatile __xdata matrix_col_t matrix[MATRIX_COLS];
__xdata matrix_col_t matrix_previous[MATRIX_COLS];
volatile bool matrix_updated;

uint8_t action_layer;

void matrix_init()
{
    current_step = 0;
    matrix_updated = false;
    action_layer = 0;

    for (int i = 0; i < MATRIX_COLS; i++) {
        matrix[i] = 0;
        matrix_previous[i] = 0;
    }
}

inline matrix_col_t matrix_get_col(uint8_t col)
{
    return matrix[col];
}

inline matrix_col_t matrix_can_read()
{
    return matrix_updated;
}

void process_key_state(uint8_t row, uint8_t col, bool pressed)
{
    uint16_t qcode = keymaps[0][row][col];

    if (IS_QK_MOMENTARY(qcode)) {
        if (pressed) {
            action_layer = QK_MOMENTARY_GET_LAYER(qcode);
        } else {
            clear_keys();
            action_layer = 0;
        }

        dprintf("CHANGED LAYER: %d\r\n", action_layer);
        return;
    }

    if (action_layer) {
        uint16_t acode = keymaps[action_layer][row][col];

        if (acode != KC_TRANSPARENT) {
            qcode = acode;
        }
    }

    if (!process_record_user(qcode, pressed)) {
        return;
    }

    if (IS_MODIFIER_KEYCODE(qcode)) {
        if (pressed) {
            add_mods(MOD_BIT((uint8_t)(qcode & 0xFF)));
        } else {
            del_mods(MOD_BIT((uint8_t)(qcode & 0xFF)));
        }

        send_keyboard_report();
        return;
    }

    if (IS_BASIC_KEYCODE(qcode)) {
        if (pressed) {
            add_key((uint8_t)(qcode & 0xFF));
        } else {
            del_key((uint8_t)(qcode & 0xFF));
        }

        send_keyboard_report();
        return;
    }

    dprintf("UNRECOGNIZED KEY: 0x%04x\r\n", qcode);
}

uint8_t matrix_task()
{
    if (!matrix_can_read()) {
        return false;
    }

    bool matrix_changed = false;

    for (uint8_t col = 0; col < MATRIX_COLS && !matrix_changed; col++) {
        matrix_changed |= matrix_previous[col] ^ matrix_get_col(col);
    }

    // Short-circuit the complete matrix processing if it is not necessary
    if (!matrix_changed) {
        matrix_updated = false;
        return matrix_changed;
    }

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        const matrix_col_t current_col = matrix_get_col(col);
        const matrix_col_t col_changes = matrix_get_col(col) ^ matrix_previous[col];

        if (!col_changes) {
            continue;
        }

        matrix_col_t row_mask = 1;

        for (uint8_t row = 0; row < MATRIX_COLS; row++, row_mask <<= 1) {
            if (col_changes & row_mask) {
                const bool key_pressed = current_col & row_mask;

                process_key_state(row, col, key_pressed);
            }
        }

        // send_keyboard_report();

        matrix_previous[col] = current_col;
    }

    matrix_updated = false;
    return matrix_changed;
}

inline void matrix_scan_step()
{
    // set all rgb sinks to low (animation step will enable needed ones)
    P0 &= ~(_P0_2 | _P0_3 | _P0_4);
    P1 &= ~(_P1_1 | _P1_2 | _P1_3);
    P4 &= ~(_P4_3 | _P4_4 | _P4_5 | _P4_6);
    P5 &= ~(_P5_7);
    P6 &= ~(_P6_1 | _P6_2 | _P6_3 | _P6_4 | _P6_5 | _P6_6 | _P6_7);

    pwm_disable();

    // ignore until matrix has been read
    if (!matrix_updated) {
        // set all columns to high
        P1 |= (uint8_t)(_P1_5);
        P2 |= (uint8_t)(_P2_0 | _P2_1 | _P2_2 | _P2_3 | _P2_4 | _P2_5);
        P3 |= (uint8_t)(_P3_0 | _P3_1 | _P3_2 | _P3_3 | _P3_4 | _P3_5);
        P5 |= (uint8_t)(_P5_0 | _P5_1 | _P5_2);

        // set current (!) column to low
        switch (current_step) {
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

        // grab key for the column state
        // P7_1 - R0
        // P7_2 - R1
        // P7_3 - R2
        // P5_3 - R3
        // P5_4 - R4
        uint8_t column_state = (((P7 >> 1) & 0x07) | (P5 & 0x18)) | 0xe0;

        // set all columns down to low
        P1 &= (uint8_t)~(_P1_5);
        P2 &= (uint8_t)~(_P2_0 | _P2_1 | _P2_2 | _P2_3 | _P2_4 | _P2_5);
        P3 &= (uint8_t)~(_P3_0 | _P3_1 | _P3_2 | _P3_3 | _P3_4 | _P3_5);
        P5 &= (uint8_t)~(_P5_0 | _P5_1 | _P5_2);

        matrix[current_step] = ~column_state;
    }

    // rgb led matrix animation
    animation_step(current_step);

    // move step
    if (current_step < MATRIX_COLS - 1) {
        current_step++;
    } else {
        current_step = 0;
        matrix_updated = true;
    }

    // clear pwm isr flag
    PWM00CON &= ~(1 << 5);

    pwm_enable();
}

void animation_step(uint8_t current_step)
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
}
