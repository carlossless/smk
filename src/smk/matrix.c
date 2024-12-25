#include "matrix.h"
#include "report.h"
#include "debug.h"
#include "layout.h"
#include "user_layout.h"
#include "kb.h"
#include "indicators.h"
#include "user_matrix.h"
#include "kbdef.h"
#include "host.h"
#include <stdlib.h>
#include <stdbool.h>

void animation_step(uint8_t current_step);

typedef uint8_t matrix_col_t;

volatile uint8_t current_step;

volatile __xdata matrix_col_t matrix[MATRIX_COLS];
__xdata matrix_col_t          matrix_previous[MATRIX_COLS];
volatile bool                 matrix_updated;

uint8_t action_layer;

void matrix_init()
{
    current_step   = 0;
    matrix_updated = false;
    action_layer   = 0;

    for (int i = 0; i < MATRIX_COLS; i++) {
        matrix[i]          = 0;
        matrix_previous[i] = 0;
    }
}

matrix_col_t matrix_get_col(uint8_t col)
{
    return matrix[col];
}

matrix_col_t matrix_can_read()
{
    return matrix_updated;
}

void process_key_state(uint8_t row, uint8_t col, bool pressed)
{
    uint16_t qcode = keymaps[0][row][col];

    // FIXME: this debug statement causes a delay which has a sideffect of fixing
    // a sticky-key problem for the last row of keys (report is unsent on key-up)
    dprintf("KEY: 0x%04x %s\r\n", qcode, pressed ? "UP" : "DOWN");

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

    if (!kb_process_record(qcode, pressed)) {
        return;
    }

    if (!layout_process_record(qcode, pressed)) {
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
    } else if (IS_SYSTEM_KEYCODE(qcode)) {
        if (pressed) {
            host_system_send(KEYCODE2SYSTEM(qcode));
        } else {
            host_system_send(0);
        }
        return;
    } else if (IS_CONSUMER_KEYCODE(qcode)) {
        if (pressed) {
            host_consumer_send(KEYCODE2CONSUMER(qcode));
        } else {
            host_consumer_send(0);
        }
        return;
    }

    dprintf("UNRECOGNIZED KEY: 0x%04x\r\n", qcode);
}

uint8_t matrix_task()
{
    if (!matrix_can_read()) {
        return false;
    }

    __xdata bool matrix_changed = false;

    for (uint8_t col = 0; col < MATRIX_COLS && !matrix_changed; col++) {
        matrix_changed |= matrix_previous[col] ^ matrix_get_col(col);
    }

    // short-circuit the complete matrix processing if it is not necessary
    if (!matrix_changed) {
        matrix_updated = false;
        return matrix_changed;
    }

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        __xdata const matrix_col_t current_col = matrix_get_col(col);
        __xdata const matrix_col_t col_changes = current_col ^ matrix_previous[col];

        if (!col_changes) {
            continue;
        }

        __xdata matrix_col_t row_mask = 1;

        for (uint8_t row = 0; row < MATRIX_ROWS; row++, row_mask <<= 1) {
            if (col_changes & row_mask) {
                __xdata const bool key_pressed = current_col & row_mask;

                process_key_state(row, col, key_pressed);
            }
        }

        matrix_previous[col] = current_col;
    }

    matrix_updated = false;
    return matrix_changed;
}

void matrix_scan_step()
{
    indicators_pre_update();

    // ignore until updated matrix has been read/flushed
    if (!matrix_updated) {
        user_matrix_pre_scan(current_step);

        uint8_t column_state = user_matrix_scan_col(current_step);

        user_matrix_post_scan();

        matrix[current_step] = ~column_state;

        // keyboard_update_switches();
    }

    // rgb led matrix animation
    indicators_update_step(&keyboard_state, current_step);

    // move step
    if (current_step < MATRIX_COLS - 1) {
        current_step++;
    } else {
        current_step   = 0;
        matrix_updated = true;
    }

    indicators_post_update();
}
