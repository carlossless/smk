#include "matrix.h"
#include "report.h"
#include "debug.h"
#include "layout.h"
#include "user_layout.h"
#include "kb.h"
#include "user_matrix.h"
#include "kbdef.h"
#include "host.h"
#include "delay.h"
#include "indicators.h"
#include "sleep.h"
#include <stdlib.h>
#include <stdbool.h>

typedef uint8_t matrix_col_t;

__xdata matrix_col_t matrix[MATRIX_COLS];
__xdata matrix_col_t matrix_previous[MATRIX_COLS];

// Set by matrix_scan_full() each time a full column sweep completes; cleared by
// matrix_task() after it has diffed the new sample against the previous one.
// Volatile because the scan writes it asynchronously from the main loop's read.
volatile bool matrix_updated;

uint8_t action_layer;

// Base layer the matrix resolves keys against when no momentary (MO) layer is
// held. Defaults to 0; a keyboard can retarget it at runtime.
__xdata uint8_t default_layer;

void matrix_init()
{
    action_layer   = 0;
    default_layer  = 0;
    matrix_updated = false;

    for (int i = 0; i < MATRIX_COLS; i++) {
        matrix[i]          = 0;
        matrix_previous[i] = 0;
    }
}

matrix_col_t matrix_get_col(uint8_t col)
{
    return matrix[col];
}

void set_default_layer(uint8_t layer)
{
    default_layer = layer;
}

void process_key_state(uint8_t row, uint8_t col, bool pressed)
{
    uint16_t qcode = keymaps[default_layer][row][col];

    if (IS_QK_MOMENTARY(qcode)) {
        if (pressed) {
            action_layer = QK_MOMENTARY_GET_LAYER(qcode);
        } else {
            clear_keys();
            action_layer = 0;
        }

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
            host_system_send(keycode_to_system(qcode));
        } else {
            host_system_send(0);
        }
        return;
    } else if (IS_CONSUMER_KEYCODE(qcode)) {
        if (pressed) {
            host_consumer_send(keycode_to_consumer(qcode));
        } else {
            host_consumer_send(0);
        }
        return;
    }

    (void)qcode; // unrecognized keycode: nothing to send
}

// Sweep the whole matrix once: for each column, select it, sample the rows
// twice with a settle delay between, and commit only if both samples agree
// (two-sample debounce). Board hooks wrap the sweep and the LED indicators are
// quiesced across it so they can't bias the row reads.
void matrix_scan_full(void)
{
    indicators_pwm_disable();

    user_matrix_sinks_off();

    user_matrix_scan_pre();
    user_matrix_cols_deselect_all();

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        user_matrix_col_select(col);

        delay_us(10); // let the row lines settle before sampling
        const uint8_t sample1 = user_matrix_read_rows();
        delay_us(10);
        const uint8_t sample2 = user_matrix_read_rows();

        if (sample1 == sample2) {
            matrix[col] = ~sample1;
        }

        user_matrix_col_deselect(col);
    }

    user_matrix_scan_post();

    indicators_pwm_enable();

    matrix_updated = true;
}

uint8_t matrix_task()
{
    if (!matrix_updated) {
        return false;
    }

    // Snapshot the scan-written matrix[], then diff it against
    // matrix_previous[]. No lock needed: each column byte reads atomically, so a
    // concurrent scan lands cleanly on one side of the read — at worst a
    // transition is split across two main-loop iterations, never lost.
    matrix_col_t snapshot[MATRIX_COLS];
    matrix_updated = false;
    for (uint8_t i = 0; i < MATRIX_COLS; i++) {
        snapshot[i] = matrix[i];
    }

    bool matrix_changed = false;

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        const matrix_col_t current_col = snapshot[col];
        const matrix_col_t col_changes = current_col ^ matrix_previous[col];
        if (!col_changes) {
            continue;
        }
        matrix_changed = true;
        sleep_note_activity(); // a key changed state; reset the inactivity timer

        matrix_col_t row_mask = 1;
        for (uint8_t row = 0; row < MATRIX_ROWS; row++, row_mask <<= 1) {
            if (col_changes & row_mask) {
                const bool key_pressed = current_col & row_mask;
                process_key_state(row, col, key_pressed);
            }
        }

        matrix_previous[col] = current_col;
    }

    return matrix_changed;
}
