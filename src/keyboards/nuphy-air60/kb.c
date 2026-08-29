#include <stdint.h>
#include <stdbool.h>
#include "keycodes.h"
#include "kbdef.h"
#include "keyboard.h"
#include "layout.h"
#include "settings.h"
#include "debug.h"
#include "report.h"
#include "usb.h"

#ifdef RF_ENABLED
#    include "rf_controller.h"
#endif

typedef enum {
    KEYBOARD_CONN_MODE_RF  = 0,
    KEYBOARD_CONN_MODE_USB = 1,
} user_keyboard_conn_mode_t;

typedef enum {
    KEYBOARD_OS_MODE_WIN = 0,
    KEYBOARD_OS_MODE_MAC = 1,
} user_keyboard_os_mode_t;

typedef struct {
    user_keyboard_conn_mode_t conn_mode;
    user_keyboard_os_mode_t   os_mode;
} user_keyboard_state_t;

volatile user_keyboard_state_t user_keyboard_state;

void kb_init()
{
    user_keyboard_state.conn_mode = CONN_MODE_SWITCH;
    user_keyboard_state.os_mode   = OS_MODE_SWITCH;

    set_default_layer(layout_os_base_layer(user_keyboard_state.os_mode == KEYBOARD_OS_MODE_MAC));

#ifdef RF_ENABLED
    rf_set_mac_mode_compat(user_keyboard_state.os_mode == KEYBOARD_OS_MODE_MAC);
#endif
}

#define SLIDER_DEBOUNCE_ITERS 256

static void kb_apply_conn_mode(user_keyboard_conn_mode_t mode)
{
    user_keyboard_state.conn_mode = mode;

    switch (mode) {
        case KEYBOARD_CONN_MODE_USB:
            dprintf("USB_MODE\r\n");
#ifdef RF_ENABLED
            rf_apply_usb_mode();
#endif
            break;
        case KEYBOARD_CONN_MODE_RF:
            dprintf("RF_MODE\r\n");
#ifdef RF_ENABLED
            rf_set_link((rf_mode_t)user_settings.rf_link);
            rf_kbd_lazy_state_init();
#endif
            break;
    }
}

static void kb_apply_os_mode(user_keyboard_os_mode_t mode)
{
    const bool is_mac = (mode == KEYBOARD_OS_MODE_MAC);

    user_keyboard_state.os_mode = mode;
    set_default_layer(layout_os_base_layer(is_mac));

    dprintf(is_mac ? "MAC_MODE\r\n" : "WIN_MODE\r\n");
#ifdef RF_ENABLED
    rf_set_mac_mode_compat(is_mac);
#endif
}

void kb_update_switches()
{
    static uint16_t conn_debounce;
    static uint16_t os_debounce;

    const uint8_t raw_conn = CONN_MODE_SWITCH;
    if (raw_conn == user_keyboard_state.conn_mode) {
        conn_debounce = 0;
    } else if (++conn_debounce >= SLIDER_DEBOUNCE_ITERS) {
        conn_debounce = 0;
        kb_apply_conn_mode((user_keyboard_conn_mode_t)raw_conn);
    }

    const uint8_t raw_os = OS_MODE_SWITCH;
    if (raw_os == user_keyboard_state.os_mode) {
        os_debounce = 0;
    } else if (++os_debounce >= SLIDER_DEBOUNCE_ITERS) {
        os_debounce = 0;
        kb_apply_os_mode((user_keyboard_os_mode_t)raw_os);
    }
}

#ifdef RF_ENABLED
static rf_mode_t kb_keycode_to_rf_mode(uint16_t keycode)
{
    switch (keycode) {
        case LNK_BT1:
            return RF_MODE_BT1;
        case LNK_BT2:
            return RF_MODE_BT2;
        case LNK_BT3:
            return RF_MODE_BT3;
        case LNK_24G:
        default:
            return RF_MODE_2_4G;
    }
}
#endif

extern void indicators_next_effect();
extern void indicators_prev_effect();
extern void indicators_brightness_up();
extern void indicators_brightness_down();
extern void indicators_speed_up();
extern void indicators_speed_down();
extern void indicators_ul_next_effect();
extern void indicators_ul_prev_effect();
extern void indicators_ul_brightness_up();
extern void indicators_ul_brightness_down();
extern void indicators_ul_speed_up();
extern void indicators_ul_speed_down();
extern void indicators_factory_reset();
extern void indicators_battery_flash();
extern void indicators_battery_on();
extern void indicators_battery_off();

static bool ul_mode_active;

static bool reset_mode_active;

#ifdef RF_ENABLED
#    define LINK_PAIRING_HOLD_TICKS 60000
static uint16_t link_hold_ticks    = 0;
static uint16_t link_hold_keycode  = 0;
static bool     link_pairing_armed = false;
#endif

bool kb_process_record(uint16_t keycode, bool key_pressed)
{
    switch (keycode) {
        case UL_MODE:
            ul_mode_active = key_pressed;
            return false;
        case RST_HLD:
            reset_mode_active = key_pressed;
            return false;
        case FCT_RST:
            if (key_pressed && reset_mode_active) {
                indicators_factory_reset();
#ifdef RF_ENABLED
                rf_factory_reset_bonds();
#endif
            }
            return false;
        case FX_NEXT:
            if (key_pressed) {
                if (ul_mode_active) {
                    indicators_ul_next_effect();
                } else {
                    indicators_next_effect();
                }
            }
            return false;
        case FX_PREV:
            if (key_pressed) {
                if (ul_mode_active) {
                    indicators_ul_prev_effect();
                } else {
                    indicators_prev_effect();
                }
            }
            return false;
        case BRI_UP:
            if (key_pressed) {
                if (ul_mode_active) {
                    indicators_ul_brightness_up();
                } else {
                    indicators_brightness_up();
                }
            }
            return false;
        case BRI_DN:
            if (key_pressed) {
                if (ul_mode_active) {
                    indicators_ul_brightness_down();
                } else {
                    indicators_brightness_down();
                }
            }
            return false;
        case SPD_UP:
            if (key_pressed) {
                if (ul_mode_active) {
                    indicators_ul_speed_up();
                } else {
                    indicators_speed_up();
                }
            }
            return false;
        case SPD_DN:
            if (key_pressed) {
                if (ul_mode_active) {
                    indicators_ul_speed_down();
                } else {
                    indicators_speed_down();
                }
            }
            return false;
#ifdef RF_ENABLED
        case LNK_BT1:
        case LNK_BT2:
        case LNK_BT3:
        case LNK_24G:
            if (user_keyboard_state.conn_mode == KEYBOARD_CONN_MODE_RF) {
                if (key_pressed) {
                    rf_mode_t mode = kb_keycode_to_rf_mode(keycode);
                    dprintf("rf link selected %02x\r\n", mode);
                    rf_set_link(mode);
                    user_settings.rf_link = (uint8_t)mode;
                    settings_mark_dirty();
                    keyboard_state.rf_link   = (uint8_t)mode;
                    keyboard_state.connected = 1;
                    keyboard_state.paired    = 1;
                    link_hold_keycode        = keycode;
                    link_hold_ticks          = 0;
                    link_pairing_armed       = true;
                } else {
                    if (link_hold_keycode == keycode) {
                        link_hold_keycode  = 0;
                        link_pairing_armed = false;
                    }
                }
            }
            return false;
        case BAT_FL:
            if (key_pressed) {
                indicators_battery_flash();
            }
            return false;
        case BAT_ON:
            if (key_pressed) {
                indicators_battery_on();
            }
            return false;
        case BAT_OFF:
            if (key_pressed) {
                indicators_battery_off();
            }
            return false;
#endif
        default:
            return true;
    }
}

void kb_send_report(__xdata report_keyboard_t *report)
{
    switch (user_keyboard_state.conn_mode) {
        case KEYBOARD_CONN_MODE_USB:
            usb_send_report(report);
            break;
#ifdef RF_ENABLED
        case KEYBOARD_CONN_MODE_RF:
            rf_send_report(report);
            break;
#endif
    }
}

void kb_send_nkro(__xdata report_nkro_t *report)
{
    switch (user_keyboard_state.conn_mode) {
        case KEYBOARD_CONN_MODE_USB:
            usb_send_nkro(report);
            break;
#ifdef RF_ENABLED
        case KEYBOARD_CONN_MODE_RF:
            rf_send_nkro(report);
            break;
#endif
    }
}

void kb_send_extra(__xdata report_extra_t *report)
{
    switch (user_keyboard_state.conn_mode) {
        case KEYBOARD_CONN_MODE_USB:
            usb_send_extra(report);
            break;
#ifdef RF_ENABLED
        case KEYBOARD_CONN_MODE_RF:
            rf_send_extra(report);
            break;
#endif
    }
}

uint16_t ticks = 0;

void kb_update()
{
#ifdef RF_ENABLED
    if (user_keyboard_state.conn_mode == KEYBOARD_CONN_MODE_RF) {
        if (link_pairing_armed && link_hold_keycode) {
            if (link_hold_ticks < LINK_PAIRING_HOLD_TICKS) {
                link_hold_ticks++;
            } else {
                rf_mode_t mode = kb_keycode_to_rf_mode(link_hold_keycode);
                dprintf("rf link pairing %02x\r\n", mode);
                keyboard_state.paired    = 0;
                keyboard_state.connected = 0;
                rf_set_link_pairing(mode, &keyboard_state);
                link_pairing_armed = false;
            }
        }

        rf_link_supervisor(&keyboard_state);

        rf_send_pending_flush();

        rf_blanking_tick();
    }
#endif
    ticks++;
}
