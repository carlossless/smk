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

void matrix_init()
{
    action_layer   = 0;
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

void process_key_state(uint8_t row, uint8_t col, bool pressed)
{
    uint16_t qcode = keymaps[0][row][col];

    // TEMP: dprintf disabled to test the stack-overflow theory. process_key_state
    // already runs ~5 frames deep from main loop, and dprintf -> printf adds a
    // heavy frame on top. With SP wrapping past 0xFF (observed: sp3, sp14, sp34
    // in the brightness handler), removing this should bring SP back into the
    // legitimate 0x85..0xFF range. If it does, the fix is to keep this off (or
    // route key-event logging through a single-frame-deep ring buffer).
    // dprintf("KEY: 0x%04x %s\r\n", qcode, pressed ? "UP" : "DOWN");

    if (IS_QK_MOMENTARY(qcode)) {
        if (pressed) {
            action_layer = QK_MOMENTARY_GET_LAYER(qcode);
        } else {
            clear_keys();
            action_layer = 0;
        }

        // dprintf("CHANGED LAYER: %d\r\n", action_layer);
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

    (void)qcode; // UNRECOGNIZED KEY: cold path, drop the dprintf to keep printf_large out of the link
}

// Called from the Timer 2 ISR. Disables the LED column PWM (so the col
// pins are GPIO-driven, not PWM-driven), turns off every LED row sink
// so leakage current through the matrix doesn't bias the row reads,
// drives the cols as outputs and HIGH, then for each of the 16 cols drops
// it LOW, samples rows twice ~10 µs apart, and only commits the result if
// both samples agree. Releases the cols back to INPUT/high-Z at the end and
// re-enables PWM. Mirrors the matrix-phase branch of stock fw's
// isr_timer2_pwm_anim, including its column DIRECTION toggle.
void matrix_scan_full(void)
{
    indicators_pwm_disable();

    // Stock-match: drop every LED row sink so no LED current flows
    // through the matrix while we sample. Without this, with a row sink
    // sourcing 3.3V and a col driven LOW for scan, the LED conducts and
    // the resulting current biases adjacent row traces.
    user_matrix_sinks_off();

    // Drive the columns as outputs for the sweep. They idle as inputs
    // (high-Z) between sweeps — see the cols_highz() release below — so
    // cols_output() must precede cols_high_all() or the HIGH wouldn't take.
    user_matrix_cols_output();
    user_matrix_cols_high_all();

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        user_matrix_col_low(col);

        // Settle. Row pin RC (pull-up + trace cap) is ~1 µs; stock's
        // PCA-timer + delay_us(0,1) totals ~3-4 µs. We use 10 µs as a
        // generous margin — net cost is 160 µs per full sweep, still
        // well under the Timer 2 period.
        delay_us(10);
        const uint8_t sample1 = user_matrix_read_rows();
        delay_us(10);
        const uint8_t sample2 = user_matrix_read_rows();

        if (sample1 == sample2) {
            matrix[col] = ~sample1;
        }

        user_matrix_col_high(col);
    }

    // Release the columns to INPUT/high-Z (stock does the same at the end of
    // its matrix subframe). This is what makes a parked PWM column non-
    // sourcing: when the LED PWM is later disabled — per-subframe, or for the
    // ~5 ms flash-erase stall — the pins float instead of holding a row
    // bright. The PWM peripheral still drives them while enabled. This is why
    // the flash-erase blank (flash.c) only needs to park the PWM, with no
    // row-sink drop, exactly like stock's led_pwm_disable_all.
    user_matrix_cols_highz();

    indicators_pwm_enable();

    matrix_updated = true;
}

uint8_t matrix_task()
{
    if (!matrix_updated) {
        return false;
    }

    // Stock-mirror three-buffer scheme (kbd_matrix_diff_apply at CODE:0x5fd9):
    //   matrix[]          = scan_curr_buf  @ 0x06B0..0x06C5  — ISR writes here.
    //   snapshot[]        = scan_prev_buf  @ 0x05DE..0x05F3  — copied below.
    //   matrix_previous[] = scan_old_buf   @ 0x0679..0x068E  — diff target.
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
