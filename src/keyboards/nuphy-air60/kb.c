#include <stdint.h>
#include <stdbool.h>
#include "keycodes.h"
#include "kbdef.h"
#include "keyboard.h"
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

volatile __xdata user_keyboard_state_t user_keyboard_state;

void kb_init()
{
    user_keyboard_state.conn_mode = CONN_MODE_SWITCH;
    user_keyboard_state.os_mode   = OS_MODE_SWITCH;
}

void kb_update_switches()
{
    if (user_keyboard_state.conn_mode != CONN_MODE_SWITCH) {
        user_keyboard_state.conn_mode = CONN_MODE_SWITCH;
        switch (user_keyboard_state.conn_mode) {
            case KEYBOARD_CONN_MODE_USB:
                dprintf("USB_MODE\r\n");
                break;
            case KEYBOARD_CONN_MODE_RF:
                dprintf("RF_MODE\r\n");
                break;
        }
    }

    if (user_keyboard_state.os_mode != OS_MODE_SWITCH) {
        user_keyboard_state.os_mode = OS_MODE_SWITCH;
        switch (user_keyboard_state.os_mode) {
            case KEYBOARD_OS_MODE_MAC:
                dprintf("MAC_MODE\r\n");
                break;
            case KEYBOARD_OS_MODE_WIN:
                dprintf("WIN_MODE\r\n");
                break;
        }
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

// While UL_MODE (the "?" key on the Fn layer) is held, the RGB_* chords adjust the
// underglow instead of the main backlight. Held in xdata to spare internal RAM.
static __xdata bool ul_mode_active;

// While RESET_HOLD (Fn+Tab) is held, pressing FACT_RESET (V) factory-resets settings.
static __xdata bool reset_mode_active;

#ifdef RF_ENABLED
// Track a LNK_BT* / LNK_24G being held so kb_update can fire the pairing
// command after a 3 s long-press. ~3 s at ~20 kHz tick rate = 60000 ticks.
#define LINK_PAIRING_HOLD_TICKS 60000
static __xdata uint16_t link_hold_ticks       = 0;
static __xdata uint16_t link_hold_keycode     = 0;
static __xdata bool     link_pairing_armed    = false;
#endif

bool kb_process_record(uint16_t keycode, bool key_pressed)
{
    switch (keycode) {
        case UL_MODE:
            ul_mode_active = key_pressed;
            return false;
        case RESET_HOLD:
            reset_mode_active = key_pressed;
            return false;
        case FACT_RESET:
            if (key_pressed && reset_mode_active) {
                indicators_factory_reset();
            }
            return false;
        case RGB_FX_NEXT:
            if (key_pressed) {
                if (ul_mode_active) {
                    indicators_ul_next_effect();
                } else {
                    indicators_next_effect();
                }
            }
            return false;
        case RGB_FX_PREV:
            if (key_pressed) {
                if (ul_mode_active) {
                    indicators_ul_prev_effect();
                } else {
                    indicators_prev_effect();
                }
            }
            return false;
        case RGB_BRI_UP:
            if (key_pressed) {
                if (ul_mode_active) {
                    indicators_ul_brightness_up();
                } else {
                    indicators_brightness_up();
                }
            }
            return false;
        case RGB_BRI_DN:
            if (key_pressed) {
                if (ul_mode_active) {
                    indicators_ul_brightness_down();
                } else {
                    indicators_brightness_down();
                }
            }
            return false;
        case RGB_SPD_UP:
            if (key_pressed) {
                if (ul_mode_active) {
                    indicators_ul_speed_up();
                } else {
                    indicators_speed_up();
                }
            }
            return false;
        case RGB_SPD_DN:
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
                    dprintf("rf link selected: %d\r\n", mode);
                    rf_set_link(mode);
                    user_settings.rf_link = (uint8_t)mode;
                    settings_save();
                    // Optimistic indicator update: assume paired+connected
                    // so the indicator stays solid until the next status
                    // poll downgrades it.
                    keyboard_state.rf_link   = (uint8_t)mode;
                    keyboard_state.connected = 1;
                    keyboard_state.paired    = 1;
                    // Arm long-press → pairing.
                    link_hold_keycode  = keycode;
                    link_hold_ticks    = 0;
                    link_pairing_armed = true;
                } else {
                    // Release — clear long-press tracker if it's for this key.
                    if (link_hold_keycode == keycode) {
                        link_hold_keycode  = 0;
                        link_pairing_armed = false;
                    }
                }
            }
            return false;
        case BAT_FLASH:
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
        // Drives the post-release blanking state machine. Internally throttled
        // — at most one SPI burst per RF_BLANKING_TICK_THROTTLE main-loop
        // iterations — so it spaces the blanking packets out enough for the
        // LED PWM ISR to scan between them.
        rf_blanking_tick();

        // Long-press LNK_*: after the key has been held continuously for
        // LINK_PAIRING_HOLD_TICKS (~3 s), enter pairing mode for that slot.
        // Fires exactly once per hold.
        if (link_pairing_armed && link_hold_keycode) {
            if (link_hold_ticks < LINK_PAIRING_HOLD_TICKS) {
                link_hold_ticks++;
            } else {
                rf_mode_t mode = kb_keycode_to_rf_mode(link_hold_keycode);
                dprintf("rf link pairing: %d\r\n", mode);
                // Show the unpaired (fast blink) state immediately. The
                // burst inside rf_set_link_pairing will overwrite this
                // with the real status as soon as the BK3632 confirms.
                keyboard_state.paired    = 0;
                keyboard_state.connected = 0;
                rf_set_link_pairing(mode, &keyboard_state);
                link_pairing_armed = false;
            }
        }

        if (ticks > 20000) {
            rf_update_keyboard_state(&keyboard_state);
            ticks = 0;
        }
    }
#endif
    ticks++;
}
