#include <stdint.h>
#include <stdbool.h>
#include "report.h"
#include "usb.h"
#include "kbdef.h"

// Runs inside the tick interrupt, where the matrix scan lives, so it must not print:
// console_printf is reentrant and far too heavy for interrupt context.
//
// While the position-discovery keymap is loaded every keycode is 0x04 + row*24 + col,
// so the position can be recovered here and accumulated. Latching every position ever
// seen means the report does not have to be read while a key is held.
#define KC_POS_BASE 0x04u

volatile uint8_t kb_seen[MATRIX_COLS];
volatile uint8_t kb_seen_seq;

bool kb_process_record(uint16_t keycode, bool key_pressed)
{
    if (key_pressed && keycode >= KC_POS_BASE && keycode < KC_POS_BASE + (MATRIX_ROWS * MATRIX_COLS)) {
        const uint8_t index = (uint8_t)(keycode - KC_POS_BASE);
        const uint8_t row   = index / MATRIX_COLS;
        const uint8_t col   = index % MATRIX_COLS;

        if ((kb_seen[col] & (uint8_t)(1u << row)) == 0) {
            kb_seen[col] |= (uint8_t)(1u << row);
            kb_seen_seq++;
        }
    }
    return true;
}

void kb_send_report(__xdata report_keyboard_t *report)
{
    usb_send_report(report);
}

void kb_send_nkro(__xdata report_nkro_t *report)
{
    usb_send_nkro(report);
}

void kb_send_extra(__xdata report_extra_t *report)
{
    usb_send_extra(report);
}
