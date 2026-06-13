#include <stdint.h>
#include <stdbool.h>
#include "keycodes.h"
#include "kbdef.h"
#include "keyboard.h"
#include "settings.h"
#include "debug.h" // dprintf
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

#ifdef RF_ENABLED
    // Prime the BK3632's Mac-compat byte9 override from the initial
    // OS_MODE_SWITCH read. Without this the first packets after boot
    // would carry the wrong byte9 marker for Mac hosts until the
    // first kb_update_switches transition fires.
    rf_set_mac_mode_compat(user_keyboard_state.os_mode == KEYBOARD_OS_MODE_MAC);
#endif
}

// Slider debouncing. Sample the CONN_MODE and OS_MODE slider pins and
// accumulate over a window before committing a transition: a per-slider
// count of consecutive disagreement, so when the raw pin reads the
// uncommitted value for SLIDER_DEBOUNCE_ITERS iterations in a row, we
// commit the new value.
//
// At our ~20 kHz main-loop rate, 256 iters ≈ 13 ms — short enough to
// feel instantaneous, long enough to filter mechanical contact noise
// during a slide.
#define SLIDER_DEBOUNCE_ITERS 256

void kb_update_switches()
{
    static __xdata uint16_t conn_debounce;
    static __xdata uint16_t os_debounce;

    // CONN_MODE_SWITCH — debounce + commit + transition
    const uint8_t raw_conn = CONN_MODE_SWITCH;
    if (raw_conn == user_keyboard_state.conn_mode) {
        conn_debounce = 0;
    } else if (++conn_debounce >= SLIDER_DEBOUNCE_ITERS) {
        conn_debounce               = 0;
        user_keyboard_state.conn_mode = raw_conn;
        switch (user_keyboard_state.conn_mode) {
            case KEYBOARD_CONN_MODE_USB:
                dprintf("USB_MODE\r\n");
#ifdef RF_ENABLED
                // Tell the BK3632 the host is now wired so it stops
                // trying to forward keys wirelessly.
                rf_apply_usb_mode();
#endif
                break;
            case KEYBOARD_CONN_MODE_RF:
                dprintf("RF_MODE\r\n");
#ifdef RF_ENABLED
                // Re-prime the BK3632 on the saved link slot.
                rf_set_link((rf_mode_t)user_settings.rf_link);
                // Lazy-init: queue a clean baseline release so the host
                // sees zero keys held after the transition, then the
                // next matrix scan re-detects whatever's actually down.
                rf_kbd_lazy_state_init();
#endif
                break;
        }
    }

    // OS_MODE_SWITCH — debounce + commit + Mac-compat byte9 update
    const uint8_t raw_os = OS_MODE_SWITCH;
    if (raw_os == user_keyboard_state.os_mode) {
        os_debounce = 0;
    } else if (++os_debounce >= SLIDER_DEBOUNCE_ITERS) {
        os_debounce               = 0;
        user_keyboard_state.os_mode = raw_os;
        switch (user_keyboard_state.os_mode) {
            case KEYBOARD_OS_MODE_MAC:
                dprintf("MAC_MODE\r\n");
#ifdef RF_ENABLED
                rf_set_mac_mode_compat(true);
#endif
                break;
            case KEYBOARD_OS_MODE_WIN:
                dprintf("WIN_MODE\r\n");
#ifdef RF_ENABLED
                rf_set_mac_mode_compat(false);
#endif
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
// One long-press = one rf_set_link_pairing call. The BK3632 advertises and
// accepts pairings for its internal window (~30 s); after that, the user
// long-presses again or selects a different slot. We deliberately fire once
// per FN-key press and don't periodically re-fire: re-firing rotates the
// BK3632's BLE advertising MAC and disrupts hosts mid-SMP.
#define LINK_PAIRING_HOLD_TICKS 60000
static __xdata uint16_t link_hold_ticks   = 0;
static __xdata uint16_t link_hold_keycode = 0;
static __xdata bool     link_pairing_armed = false;
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
#ifdef RF_ENABLED
                // Also wipe the BK3632's BT bond storage and re-init it.
                // Recovers from the BLE-stuck-rotating-MAC failure mode.
                rf_factory_reset_bonds();
#endif
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
                    dprintf("rf link selected %02x\r\n", mode);
                    rf_set_link(mode);
                    user_settings.rf_link = (uint8_t)mode;
                    settings_mark_dirty();
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

__xdata uint16_t ticks = 0;

void kb_update()
{
#ifdef RF_ENABLED
    if (user_keyboard_state.conn_mode == KEYBOARD_CONN_MODE_RF) {
        // Long-press LNK_*: after the key has been held continuously for
        // LINK_PAIRING_HOLD_TICKS (~3 s), enter pairing mode for that slot.
        // Fires exactly once per hold.
        if (link_pairing_armed && link_hold_keycode) {
            if (link_hold_ticks < LINK_PAIRING_HOLD_TICKS) {
                link_hold_ticks++;
            } else {
                rf_mode_t mode = kb_keycode_to_rf_mode(link_hold_keycode);
                dprintf("rf link pairing %02x\r\n", mode);
                // Show the unpaired (fast blink) state immediately. The
                // burst inside rf_set_link_pairing will overwrite this
                // with the real status as soon as the BK3632 confirms.
                keyboard_state.paired    = 0;
                keyboard_state.connected = 0;
                rf_set_link_pairing(mode, &keyboard_state);
                link_pairing_armed = false;
            }
        }

        // The supervisor handles status polling at ~10 polls/s and, like
        // stock, re-asserts the commanded link on every poll while the
        // BK3632 reports a dead or mismatched link.
        rf_link_supervisor(&keyboard_state);

        // Send-pending retry. rf_send_report queues the 6KRO snapshot
        // and attempts an immediate send; if the BK3632 didn't ack,
        // rf_pending stays set and we re-attempt here on every loop tick
        // until ACK lands.
        rf_send_pending_flush();

        // Post-release blanking: if rf_send_kro_report just observed a
        // release transition, drain the queued blank packets one per
        // loop tick. Without this, a dropped release packet (BK3632
        // unack or host RF drop) leaves the key latched on the host.
        rf_blanking_tick();
    }
#endif
    ticks++;
}
