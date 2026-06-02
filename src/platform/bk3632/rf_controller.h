#pragma once

#include "report.h"
#include "keyboard.h"

#include "utils/8051_inc_error_hide.h"

typedef enum {
    RF_MODE_2_4G = 0x00,
    RF_MODE_BT1  = 0x01,
    RF_MODE_BT2  = 0x02,
    RF_MODE_BT3  = 0x03
} rf_mode_t;

void rf_init();
void rf_send_report(__xdata report_keyboard_t *report);
void rf_send_nkro(__xdata report_nkro_t *report);
void rf_send_extra(__xdata report_extra_t *report);
void rf_update_keyboard_state(keyboard_state_t *keyboard);
void rf_set_link(rf_mode_t link);
// Switch to `link` and tell the BK3632 to start advertising / accepting new
// pairings on that slot. Triggered by a 3-second hold of the link key.
// After the pairing command, aggressively poll status so the BK3632's
// pairing-complete signal is consumed quickly and the chip doesn't sit in a
// half-finished state. `keyboard` is updated with whatever the poll observes.
void rf_set_link_pairing(rf_mode_t link, __xdata keyboard_state_t *keyboard);
// Wipes BK3632 BT bonds and re-initializes BT names. Use as a recovery
// path when BLE pairing keeps failing (BK3632 stuck in rotating-MAC mode).
void rf_factory_reset_bonds(void);
// Periodic supervisor — call from the main loop on every tick. Internally
// rate-limits to roughly the cadence stock fw uses (~100 ms). Stock's
// equivalent is rf_link_supervisor (FUN_CODE_986f) at CODE:986f, which runs
// off a 0x13-tick counter inside the main loop. Performs status polling,
// keeps the BK3632's saved link mode primed (mirrors stock's FUN_CODE_80a8
// transition guard), and consumes any pending pairing event.
void rf_link_supervisor(keyboard_state_t *keyboard);
// Re-fire saved link mode (no pairing flag) — used to re-prime the BK3632
// after pair-complete and as a periodic keep-alive nudge from the
// supervisor. Stock's mode_detect fires this twice in a row; we do the
// same here.
void rf_reassert_link(rf_mode_t link);

// Tell the BK3632 the host is now USB-driven (the keyboard switched to
// wired mode via the slider). Fires CMD_06 with arg=1 twice with a
// delay between, matching the USB-entry branch of stock fw's
// rf_signal_quality_probe at CODE:0x80a8 — the BK3632 then stops
// trying to forward keys to the wireless host and goes quiet.
void rf_apply_usb_mode(void);

// Mac OS host needs every CMD_REPORT to carry the "active" marker (byte9
// = 0) — when byte9 = 1 (idle), the BK3632 dedupes those packets and
// macOS misses release events. Driven by the OS_MODE_SWITCH slider.
//
// Internally this sets stock fw's `byte9_force_zero_flag` at XRAM 0x0b05
// (rf_signal_quality_probe at CODE:0x80a8 — what the Ghidra dump's
// "signal strength" comment was actually reading is the OS_MODE_SWITCH
// pin at P5.6 over a debounce window).
void rf_set_mac_mode_compat(bool is_mac);

// byte9_disable_flag override (XRAM 0x03a8). When set, the byte9 = 1
// (idle marker) branch in rf_send_kro_report is skipped — byte9 stays
// 0 even on idle packets. Stock sets this in some transitional paths
// we haven't fully traced; exposed for future use, not currently driven.
void rf_byte9_set_disable(bool on);

// Mirror of stock fw's kbd_lazy_state_init at CODE:0xaf15. On the next
// rf_send_kro_report call, zero the active key state on the wire and
// send a clean baseline release so the host's HID state aligns with
// ours after a mode change / wake-from-sleep / config reset. Driven
// externally (e.g. the USB↔Wireless slider handler).
void rf_kbd_lazy_state_init(void);

// Pending-send queue (stock-style b0c8_state_byte at XRAM:0x05ae). The
// matrix dispatch path calls rf_send_report which stashes the snapshot
// into rf_pending_buf and tries to send immediately. If the send fails
// after RF_SEND_MAX_ATTEMPTS retries the pending flag stays set, and
// rf_send_pending_flush re-attempts from the same buffer on each
// main-loop iteration until success. Each new matrix event overwrites
// the buffer with the latest state.
void rf_send_pending_flush(void);

// Post-release blanking. Stock fw's rf_send_kro_report (CODE:0x867a)
// includes a state machine (around 0x86cd..0x86fa, gated on BIT _a_6 /
// _a_7) that, after a release-transition packet, fires one phantom
// packet (HID key 0x01 = ErrorRollOver) followed by up to 3 all-zero
// blanking packets. Without this redundancy a single dropped release
// packet leaves the host with the key permanently latched.
//
// Call rf_blanking_tick() once per main-loop iteration when in RF mode
// — if there's a pending blanking packet, it sends one and decrements
// the counter. A new keypress between blanking iterations cancels any
// pending blanking automatically (rf_send_kro_report resets the
// counter when it sees an active state).
void rf_blanking_tick(void);
