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
// magic + checksum alone; status byte 0 bit 7 is the "awake" marker and only
// triggers a wake-nudge — it does not invalidate the frame.
bool rf_update_keyboard_state(keyboard_state_t *keyboard);
void rf_set_link(rf_mode_t link);
// Switch to `link` and tell the BK3632 to start advertising / accepting new
// pairings on that slot. Then aggressively poll status so the pairing-complete
// signal is consumed quickly. `keyboard` is updated with what the poll observes.
void rf_set_link_pairing(rf_mode_t link, __xdata keyboard_state_t *keyboard);
// Wipe BT bonds and re-init BT names. Recovery path for when BLE pairing keeps
// failing (BK3632 stuck in rotating-MAC mode).
void rf_factory_reset_bonds(void);
// Periodic supervisor — call from the main loop every tick (rate-limits
// internally). While the BK3632 reports neither connected nor paired (or a
// link mode other than commanded), re-fire set_link(commanded, 0) to kick it
// back into operational state, clearing the bogus "pairing blink" after a power
// cycle. Suppressed while an rf_set_link_pairing is pending.
void rf_link_supervisor(keyboard_state_t *keyboard);
// Re-fire the commanded link mode (no pairing flag) twice — re-primes the
// BK3632 after pair-complete.
void rf_reassert_link(rf_mode_t link);

// Tell the BK3632 the host is now USB-driven. Fires CMD_06 arg=1 twice; the
// BK3632 then stops forwarding keys to the wireless host and goes quiet.
void rf_apply_usb_mode(void);

// macOS needs every CMD_REPORT flagged active (byte9 = 0); on idle (byte9 = 1)
// the BK3632 dedupes and macOS misses releases. Driven by OS_MODE_SWITCH.
void rf_set_mac_mode_compat(bool is_mac);

// When set, byte9 stays 0 even on idle packets. Exposed for future use.
void rf_byte9_set_disable(bool on);

// On the next rf_send_kro_report, zero the key state on the wire and send a
// clean baseline release so the host's HID state re-aligns with ours after a
// mode change / wake-from-sleep / config reset.
void rf_kbd_lazy_state_init(void);

// Pending-send queue. rf_send_report stashes the snapshot and sends
// immediately; if that fails after RF_SEND_MAX_ATTEMPTS the pending flag stays
// set and rf_send_pending_flush re-attempts each main-loop iteration. Each new
// matrix event overwrites the buffer.
void rf_send_pending_flush(void);

// Post-release blanking: after a release packet, fire 3 phantom packets (HID
// key 0x01 = ErrorRollOver) interleaved with 3 all-zero blanks, so a single
// dropped release doesn't leave the key latched on the host. Call once per
// main-loop iteration in RF mode; a new keypress cancels any pending blanking.
void rf_blanking_tick(void);

// BK3632 sleep/wake (CMD_07 / CMD_0B) for the sleep feature: rf_prepare_sleep
// quiets the BK3632 so it stops toggling its ACK line (P4.2 / INT42, an INT4
// source that would otherwise re-wake the MCU); rf_wake_from_sleep brings it
// back on MCU wake.
void rf_prepare_sleep(uint8_t param);
void rf_wake_from_sleep(void);
// BK3632 wake-nudge (CMD_0C). Re-syncs the BK3632 after the MCU resumes.
void rf_wake_nudge(void);
