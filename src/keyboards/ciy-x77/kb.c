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

static __bit keyboard_locked;
static __bit gui_locked;
static __bit wasd_swapped;

// swapped both ways at once, so the arrows type WASD while WASD moves the cursor
static uint8_t wasd_swap(uint16_t keycode)
{
    switch (keycode) {
        case KC_W:
            return KC_UP;
        case KC_S:
            return KC_DOWN;
        case KC_A:
            return KC_LEFT;
        case KC_D:
            return KC_RGHT;
        case KC_UP:
            return KC_W;
        case KC_DOWN:
            return KC_S;
        case KC_LEFT:
            return KC_A;
        case KC_RGHT:
            return KC_D;
        default:
            return 0;
    }
}

// changing report format mid-stream strands whatever the host still thinks is held
static void release_everything(void)
{
    clear_keys();
    clear_mods();
    send_keyboard_report();
}

bool kb_process_record(uint16_t keycode, bool key_pressed)
{
    // the toggles come first so the lock can always be undone
    switch (keycode) {
        case KB_LOCK:
            if (key_pressed) {
                keyboard_locked = !keyboard_locked;
                release_everything();
            }
            return false;
        case GUI_LOCK:
            if (key_pressed) {
                gui_locked = !gui_locked;
            }
            return false;
        case NKRO_TG:
            if (key_pressed) {
                release_everything();
                keymap_config.nkro = !keymap_config.nkro;
            }
            return false;
        case WASD_TG:
            if (key_pressed) {
                wasd_swapped = !wasd_swapped;
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

    if (keyboard_locked) {
        return false;
    }

    // press only: a release still has to clear a Gui held from before the toggle
    if (gui_locked && key_pressed && (keycode == KC_LGUI || keycode == KC_RGUI || keycode == KC_APP)) {
        return false;
    }

    if (wasd_swapped) {
        uint8_t swapped = wasd_swap(keycode);
        if (swapped) {
            if (key_pressed) {
                add_key(swapped);
            } else {
                del_key(swapped);
            }
            send_keyboard_report();
            return false;
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
