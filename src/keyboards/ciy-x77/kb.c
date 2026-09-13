#include <stdint.h>
#include <stdbool.h>
#include "report.h"
#include "usb.h"
#include "keyboard.h"
#include "kbdef.h"

extern void indicators_next_effect();
extern void indicators_set_effect(uint8_t fx);
extern void indicators_step_brightness(bool up);
extern void indicators_step_speed(bool up);
extern void indicators_step_color(bool forward);
extern void indicators_factory_reset();

static __bit gui_locked;

bool kb_process_record(uint16_t keycode, bool key_pressed)
{
    switch (keycode) {
        case GUI_LOCK:
            if (key_pressed) {
                gui_locked = !gui_locked;
            }
            return false;
        case FX_NEXT:
            if (key_pressed) {
                indicators_next_effect();
            }
            return false;
        case FX_SET_0:
        case FX_SET_1:
        case FX_SET_2:
        case FX_SET_3:
        case FX_SET_OFF:
            if (key_pressed) {
                indicators_set_effect((uint8_t)(keycode - FX_SET_0));
            }
            return false;
        case BRI_UP:
        case BRI_DN:
            if (key_pressed) {
                indicators_step_brightness(keycode == BRI_UP);
            }
            return false;
        case SPD_UP:
        case SPD_DN:
            if (key_pressed) {
                indicators_step_speed(keycode == SPD_UP);
            }
            return false;
        case CLR_FWD:
        case CLR_BAK:
            if (key_pressed) {
                indicators_step_color(keycode == CLR_FWD);
            }
            return false;
        case FX_RST:
            if (key_pressed) {
                indicators_factory_reset();
            }
            return false;
        default:
            break;
    }

    // press only: a release still has to clear a Gui held from before the toggle
    if (gui_locked && key_pressed && (keycode == KC_LGUI || keycode == KC_RGUI || keycode == KC_APP)) {
        return false;
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
