#include <stdint.h>
#include <stdbool.h>
#include "report.h"
#include "usb.h"
#include "kbdef.h"

extern void indicators_cycle_effect();

bool kb_process_record(uint16_t keycode, bool key_pressed)
{
    switch (keycode) {
        case RGB_FX:
            if (key_pressed) {
                indicators_cycle_effect();
            }
            return false;
        default:
            return true;
    }
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
