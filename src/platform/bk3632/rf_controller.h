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
// Poll the BK3632 once and apply the status frame to `keyboard`. Returns
// true when a fresh, checksum-valid frame was applied. A frame is valid on
// magic + checksum alone; status byte 0 bit 7 is the chip's "awake" marker
// and only triggers a wake-nudge — it does not invalidate the frame.
bool rf_update_keyboard_state(keyboard_state_t *keyboard);
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
// rate-limits to ~100 ms. On every valid status poll, while the BK3632
// reports neither connected nor paired (or a link mode other than the one
// we commanded), re-fire set_link(commanded, 0) to kick it back into
// operational state. This clears the bogus "pairing blink" after a power
// cycle where the chip re-links to the dongle on its own but keeps reporting
// unpaired. Suppressed while a pairing started by rf_set_link_pairing is
// pending.
void rf_link_supervisor(keyboard_state_t *keyboard);
// Re-fire saved link mode (no pairing flag) twice — used to re-prime the
// BK3632 after pair-complete.
void rf_reassert_link(rf_mode_t link);

// Tell the BK3632 the host is now USB-driven (the keyboard switched to
// wired mode via the slider). Fires CMD_06 with arg=1 twice with a delay
// between — the BK3632 then stops trying to forward keys to the wireless
// host and goes quiet.
void rf_apply_usb_mode(void);

// Mac OS host needs every CMD_REPORT to carry the "active" marker (byte9
// = 0) — when byte9 = 1 (idle), the BK3632 dedupes those packets and
// macOS misses release events. Driven by the OS_MODE_SWITCH slider (P5.6).
void rf_set_mac_mode_compat(bool is_mac);

// byte9_disable override. When set, the byte9 = 1 (idle marker) branch in
// rf_send_kro_report is skipped — byte9 stays 0 even on idle packets.
// Exposed for future use, not currently driven.
void rf_byte9_set_disable(bool on);

// On the next rf_send_kro_report call, zero the active key state on the
// wire and send a clean baseline release so the host's HID state aligns
// with ours after a mode change / wake-from-sleep / config reset. Driven
// externally (e.g. the USB↔Wireless slider handler).
void rf_kbd_lazy_state_init(void);

// Pending-send queue. The matrix dispatch path calls rf_send_report which
// stashes the snapshot into rf_pending_buf and tries to send immediately.
// If the send fails after RF_SEND_MAX_ATTEMPTS retries the pending flag
// stays set, and rf_send_pending_flush re-attempts from the same buffer on
// each main-loop iteration until success. Each new matrix event overwrites
// the buffer with the latest state.
void rf_send_pending_flush(void);

// Post-release blanking: after a release-transition packet, fire 3 phantom
// packets (HID key 0x01 = ErrorRollOver) interleaved with 3 all-zero blanks.
// Without this redundancy a single dropped release packet leaves the host
// with the key permanently latched.
//
// Call rf_blanking_tick() once per main-loop iteration when in RF mode
// — if there's a pending blanking packet, it sends one and decrements
// the counter. A new keypress between blanking iterations cancels any
// pending blanking automatically (rf_send_kro_report resets the
// counter when it sees an active state).
void rf_blanking_tick(void);

// BK3632 sleep/wake (CMD_07 / CMD_0B). Used by the sleep feature when the MCU
// enters Power-Down: rf_prepare_sleep quiets the BK3632 so it stops toggling
// its ACK line (P4.2 / INT42 — an INT4 source that would otherwise re-wake the
// MCU the moment sleep is armed); rf_wake_from_sleep brings it back on MCU wake.
void rf_prepare_sleep(uint8_t param);
void rf_wake_from_sleep(void);
// BK3632 wake-nudge (CMD_0C). Used to re-sync the BK3632 after the MCU
// resumes by looping nudge + status-poll until the link reports connected.
void rf_wake_nudge(void);
