#include <stdint.h>
#include <stdbool.h>
#include "report.h"
#include "usb.h"
#include "kbdef.h"

extern void indicators_next_effect();
extern void indicators_factory_reset();

// While RESET_HOLD (Fn+Tab) is held, pressing FACT_RESET (V) factory-resets settings.
static __xdata bool reset_mode_active;

bool kb_process_record(uint16_t keycode, bool key_pressed)
{
    switch (keycode) {
        case RGB_FX_NEXT:
            if (key_pressed) {
                indicators_next_effect();
            }
            return false;
        case RESET_HOLD:
            reset_mode_active = key_pressed;
            return false;
        case FACT_RESET:
            if (key_pressed && reset_mode_active) {
                indicators_factory_reset();
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
