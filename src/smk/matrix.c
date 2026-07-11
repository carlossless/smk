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

// Set by matrix_scan_full() (called from the Timer 2 ISR) every time a
// full 16-column sweep completes. Cleared by matrix_task() after it has
// diffed the new sample against the previous one. Volatile because the
// ISR writes it asynchronously from the main loop's read.
volatile bool matrix_updated;

uint8_t action_layer;

// Base layer the matrix resolves keys against when no momentary (MO) layer is
// held. Defaults to 0; a keyboard can retarget it at runtime (e.g. the
// nuphy-air60 points it at _MAC_BL / _WIN_BL from the OS_MODE_SWITCH slider).
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

// Called from the Timer 2 ISR. Disables the LED column PWM (so the col
// pins are GPIO-driven, not PWM-driven), turns off every LED row sink
// so leakage current through the matrix doesn't bias the row reads, runs
// the board's per-sweep pre-hook, drives the cols HIGH, then for each of the
// 16 cols drops it LOW, samples rows twice ~10 µs apart, and only commits the
// result if both samples agree. Runs the board's per-sweep post-hook and
// re-enables PWM.
void matrix_scan_full(void)
{
    indicators_pwm_disable();

    // Drop every LED row sink so no LED current flows through the matrix while
    // we sample. Otherwise a row sink sourcing 3.3V with a col driven LOW lets
    // the LED conduct, biasing adjacent row traces.
    user_matrix_sinks_off();

    // Per-sweep board hook: prepare the column pins before we drive them — e.g.
    // nuphy-air60 switches its muxed columns from their high-Z rest state to
    // output, so it must run before cols_deselect_all() takes effect. No-op on
    // boards whose columns are permanently outputs.
    user_matrix_scan_pre();
    user_matrix_cols_deselect_all();

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        user_matrix_col_select(col);

        // Settle. Row pin RC (pull-up + trace cap) is ~1 µs; 10 µs is a
        // generous margin — net cost is 160 µs per full sweep, still well
        // under the Timer 2 period.
        delay_us(10);
        const uint8_t sample1 = user_matrix_read_rows();
        delay_us(10);
        const uint8_t sample2 = user_matrix_read_rows();

        if (sample1 == sample2) {
            matrix[col] = ~sample1;
        }

        user_matrix_col_deselect(col);
    }

    // Per-sweep board hook: restore the column pins after the sweep — e.g.
    // nuphy-air60 releases its columns to input/high-Z so that when the LED PWM
    // is later parked (per-subframe, or for the ~5 ms settings-save) the pins
    // float instead of holding a row bright. No-op on boards that keep their
    // columns driven.
    user_matrix_scan_post();

    indicators_pwm_enable();

    matrix_updated = true;
}

uint8_t matrix_task()
{
    if (!matrix_updated) {
        return false;
    }

    // Three-buffer diff: snapshot the ISR-written matrix[], then diff it
    // against matrix_previous[].
    //
    // No EA toggle around the copy — each matrix[col] is one byte and the
    // 8051 MOV is atomic per byte, so a concurrent ISR scan can land
    // entirely on either side of the read without tearing. If the ISR
    // updates matrix[col] mid-snapshot, the diff (snapshot ^ matrix_previous)
    // still catches whatever transition happened — it just may split
    // press/release across two main-loop iterations instead of one.
    __xdata matrix_col_t snapshot[MATRIX_COLS];
    matrix_updated = false;
    for (uint8_t i = 0; i < MATRIX_COLS; i++) {
        snapshot[i] = matrix[i];
    }

    __xdata bool matrix_changed = false;

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        __xdata const matrix_col_t current_col = snapshot[col];
        __xdata const matrix_col_t col_changes = current_col ^ matrix_previous[col];
        if (!col_changes) {
            continue;
        }
        matrix_changed = true;
        // A key changed state — reset the sleep inactivity timer. Placed here
        // (not behind a separate `if (matrix_changed)`) so it stays a plain
        // statement when SLEEP_ENABLE is off and the macro is a no-op.
        sleep_note_activity();

        __xdata matrix_col_t row_mask = 1;
        for (uint8_t row = 0; row < MATRIX_ROWS; row++, row_mask <<= 1) {
            if (col_changes & row_mask) {
                __xdata const bool key_pressed = current_col & row_mask;
                process_key_state(row, col, key_pressed);
            }
        }

        matrix_previous[col] = current_col;
    }

    return matrix_changed;
}
