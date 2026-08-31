#include <stdint.h>
#include <stdbool.h>
#include "report.h"
#include "usb.h"
#include "kbdef.h"
#include "debug.h"

// Runs inside the tick interrupt, where the matrix scan lives, so it must not print:
// console_printf is reentrant and far too heavy for interrupt context. Latch instead and
// let the main-loop diagnostic report it.
volatile uint16_t kb_last_keycode;
volatile uint8_t  kb_last_pressed;
volatile uint8_t  kb_event_seq;

bool kb_process_record(uint16_t keycode, bool key_pressed)
{
    kb_last_keycode = keycode;
    kb_last_pressed = key_pressed ? 1 : 0;
    kb_event_seq++;
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
