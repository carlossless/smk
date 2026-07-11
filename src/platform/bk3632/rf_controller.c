#include "rf_controller.h"
#include <stdint.h>
#include "delay.h"
#include "debug.h"    // dprintf
#include "bb_spi.h"   // FIXME: should be conditional?
#include "sh68f90a.h" // for EA

#define MAGIC_BYTE 0xaa
#define CMD_REPORT 0x02

typedef enum {
    RF_PAIRING_OFF = 0x00,
    RF_PAIRING_ON  = 0x01
} rf_pairing_t;

typedef enum {
    RF_SET_NAME_BT5 = 0x00,
    RF_SET_NAME_BT3 = 0x01
} rf_set_name_t;

// Arrays (not pointers) so the data lives entirely in __code; a
// `__code const char *` would put the pointer itself in DSEG, eating iRAM
// we want for the stack.
static const __code char rf_bt5_name[] = "SMK BT5.0";
static const __code char rf_bt3_name[] = "SMK BT3.0";

__xdata uint8_t rf_tx_buf[32];

// Tracks whether the previous CMD_REPORT packet carried any key activity
// (used to detect release transitions for blanking) and how many
// post-release blanking packets are still queued. Defined here at file
// scope so both rf_send_kro_report (which sets them) and
// rf_blanking_tick (which drains the counter) can reach them without
// forward-declare gymnastics.
static __xdata uint8_t kro_prev_active;
static __xdata uint8_t blanking_pending;
static __xdata bool    blanking_active;

// byte9 override flags.
//
//   mac_mode_compat: when 1, byte9 is unconditionally 0 ("active"
//     marker). macOS's HID handling needs every packet flagged active —
//     when byte9 = 1 (idle), the BK3632 dedupes the packet and the host
//     misses release events. Driven by the OS_MODE_SWITCH slider (P5.6).
//
//   byte9_disable: when 1, the byte9 = 1 (idle) branch is skipped.
//     Exposed for future use, not currently driven.
static __xdata bool mac_mode_compat;
static __xdata bool byte9_disable;

void rf_set_mac_mode_compat(bool is_mac)
{
    mac_mode_compat = is_mac;
}

void rf_byte9_set_disable(bool on)
{
    byte9_disable = on;
}

// Compute rf_tx_buf[9] from the CURRENT packet's activity:
//
//   base: byte9 = (curr_active == 0) ? 1 : 0
//   if (byte9_disable):    byte9 stays 0 even on idle
//   if (mac_mode_compat):  byte9 stays 0 always
static uint8_t compute_byte9(bool curr_active)
{
    if (mac_mode_compat) return 0;
    if (curr_active) return 0;
    if (byte9_disable) return 0;
    return 1;
}

bool rf_get_status(uint8_t status_bytes[2]);
void rf_set_link_mode(uint8_t mode, uint8_t pairing);
bool rf_send_kro_report(uint8_t *buffer);
void rf_send_nkro_report(__xdata uint8_t mods, __xdata uint8_t *nkro_buffer);
// BK3632 BT state-management command (internal — public callers go through
// rf_factory_reset_bonds).
//   arg=2: wipe all BT bonds (factory-reset the BK3632's pairing storage);
//          must be followed by rf_init to reload the BT names the wipe clears.
//   arg=3: separate "rebind/reconnect" branch, semantics not nailed down.
//          smk doesn't use this.
void rf_cmd_03(uint8_t param);
// Commit/apply settings — "finalise the pending config" signal to the BK3632.
// Only meaningful immediately after a rf_set_bt_name.
void rf_cmd_04();
void rf_send_consumer_system(uint16_t consumer, uint16_t system);
// USB-mode signal (arg=1 when the keyboard is in wired mode). Exact protocol
// unclear.
void    rf_cmd_06(uint8_t param);
void    rf_prepare_sleep(uint8_t param);
void    rf_set_bt_name(uint8_t type, char *name);
void    rf_query_status();
void    rf_wake_from_sleep();
void    rf_wake_nudge();
void    rf_fetch_4();
uint8_t checksum(uint8_t *data, int len);

void rf_init()
{
    __xdata uint8_t status_bytes[2];

    // 6 × delay_ms(255) before talking to the BK3632: it takes ~1.5 s after
    // VCC stabilizes to be ready for SPI, and talking to it before then
    // results in dropped commands.
    delay_ms(255);
    delay_ms(255);
    delay_ms(255);
    delay_ms(255);
    delay_ms(255);
    delay_ms(255);

    // Wait for the BK3632 to be ready: up to 10 attempts of wake-nudge +
    // status query, decreasing delay each pass. Break when the BK3632
    // reports the ack flag set in status byte 0.
    for (uint8_t tries = 10; tries > 0; tries--) {
        rf_wake_nudge();
        delay_ms(tries);
        if (rf_get_status(status_bytes) && (status_bytes[0] & 0x80)) {
            break;
        }
    }

    // Register both BT names: SET_BT5 -> 50 ms -> SET_BT3 -> 5 ms.
    // The 50 ms gap gives the BK3632 enough time to fully commit the BT5
    // name before the BT3 packet arrives. With < ~50 ms here, the BT3
    // commit silently fails and the chip falls back to its default name
    // ("Air60 BT3.0"). rf_set_bt_name internally does the 20 ms
    // SET -> COMMIT delay.
    rf_set_bt_name(RF_SET_NAME_BT5, rf_bt5_name);
    delay_ms(50);
    rf_set_bt_name(RF_SET_NAME_BT3, rf_bt3_name);
    delay_ms(5);

    // The BK3632 needs a follow-up SPI packet within milliseconds of the BT3
    // commit to actually persist the BT3 name slot — without it the second
    // commit silently drops. main() later calls rf_set_link again with the
    // saved link from flash, so this default call is just to flush the commit.
    rf_set_link(RF_MODE_2_4G);
}

// Re-fire rf_set_link_mode(mode, 0) twice with a small gap. The double-fire
// is what locks the BK3632 into the requested link slot — a single shot is
// sometimes silently dropped if the chip is mid-state-transition
// (advertising → connected → bonded).
void rf_reassert_link(rf_mode_t link)
{
    rf_set_link_mode((uint8_t)link, 0);
    delay_ms(20);
    rf_set_link_mode((uint8_t)link, 0);
}

// "Wipe all BT bonds" / factory reset. This is destructive: it tears down
// BLE advertising and any committed BT names, so we must re-run rf_init
// afterwards to reload them. Recovers a BK3632 that's accumulated bad BLE
// state across repeated failed pairings (which manifests as rotating-MAC
// advertising that the host can't pair against because SMP keeps timing out).
void rf_factory_reset_bonds(void)
{
    // 1. Wipe all stored bonds in the BK3632.
    rf_cmd_03(2);
    delay_ms(200);
    // 2. Force a BK3632 internal-state reset by putting it to sleep then
    //    waking, re-running its own state machine from scratch.
    rf_prepare_sleep(0);
    delay_ms(100);
    rf_wake_from_sleep();
    delay_ms(200);
    // 3. Reload BT names (the wipe + sleep cycle clears them).
    rf_init();
    delay_ms(200);
}

__xdata uint8_t kro6buffer[6];

// Send queue. When rf_send_report is called from the matrix dispatch path,
// it stashes the 6KRO snapshot here and tries to send immediately. If the
// immediate send fails (BK3632 unresponsive after RF_SEND_MAX_ATTEMPTS
// retries), rf_pending stays set; the next pass through kb_update fires
// rf_send_pending_flush which retries from the same buffer. Each new
// matrix event overwrites the buffer with the latest state so retries
// always converge to the current truth.
static __xdata uint8_t rf_pending_buf[6];
static __xdata bool    rf_pending;

void rf_send_report(__xdata report_keyboard_t *report)
{
    rf_pending_buf[0] = report->raw[0];
    rf_pending_buf[1] = report->raw[2];
    rf_pending_buf[2] = report->raw[3];
    rf_pending_buf[3] = report->raw[4];
    rf_pending_buf[4] = report->raw[5];
    rf_pending_buf[5] = report->raw[6];
    rf_pending        = true;

    rf_send_pending_flush();
}

void rf_send_pending_flush(void)
{
    if (!rf_pending) return;
    // rf_send_kro_report itself uses rf_send_or_retry; if every retry
    // returns no-ACK we leave rf_pending set so kb_update's next call
    // re-enters.
    if (rf_send_kro_report(rf_pending_buf)) {
        rf_pending = false;
    }
}

void rf_send_nkro(__xdata report_nkro_t *report)
{
    __xdata bool blank = true;
    for (__xdata int i = 1; i < NKRO_REPORT_SIZE - 1; i++) {
        if (report->raw[i] != 0) {
            blank = false;
        }
    }

    if (blank) {
        // NKRO release: re-use the 6-byte cmd-02 buffer with all keys zero;
        // rf_send_kro_report's state machine writes the right rf_tx_buf[9].
        for (__xdata int i = 0; i < 6; i++) {
            kro6buffer[i] = 0;
        }
        rf_send_kro_report(kro6buffer);
    } else {
        rf_send_nkro_report(report->mods, report->bits);
    }
}

void rf_send_extra(__xdata report_extra_t *report)
{
    switch (report->report_id) {
        case REPORT_ID_SYSTEM:
            rf_send_consumer_system(0, report->usage);
            break;
        case REPORT_ID_CONSUMER:
            rf_send_consumer_system(report->usage, 0);
            break;
    }
}

bool rf_update_keyboard_state(keyboard_state_t *keyboard)
{
    __xdata uint8_t status_bytes[2];

    // Single-shot status query. rf_query_status (send) runs with EA enabled;
    // rf_fetch_4 brackets just its receive burst with EA=0. The LED scan
    // therefore keeps running through the send and only pauses for the
    // ~600 µs receive — invisible.
    //
    // On bad magic / checksum we skip the poll; rf_get_status sends
    // rf_wake_nudge on its way out so the next poll cycle starts with the
    // BK3632 already nudged awake.
    if (!rf_get_status(status_bytes)) {
        return false; // no fresh status — leave keyboard_state untouched
    }

    // status_bytes[0] bit 7 is the BK3632's "awake" marker. A frame is valid
    // on magic + checksum alone and is still applied when this bit is clear;
    // the bit only triggers a wake-nudge. Discarding such frames would freeze
    // the displayed link state (e.g. paired=0 captured mid-reconnect after a
    // power cycle) while the keyboard kept working.
    if (!(status_bytes[0] & 0x80)) {
        rf_wake_nudge();
    }

    // status_bytes[0]: bits 0-2 = battery level (0..7), bit 7 = awake marker
    keyboard->battery_level = status_bytes[0] & 0x07;

    // status_bytes[1]: bits 0-2 = num/caps/scroll LEDs, bit 3 = connected,
    //                  bit 4 = paired, bits 5-6 = RF mode, bit 7 = low-power
    keyboard->led_state = status_bytes[1] & ((1 << 0) | (1 << 1) | (1 << 2));
    keyboard->connected = (status_bytes[1] >> 3) & 1;
    keyboard->paired    = (status_bytes[1] >> 4) & 1;
    keyboard->low_power = (status_bytes[1] >> 7) & 1;

    uint8_t old_rf_link = keyboard->rf_link;
    keyboard->rf_link   = ((status_bytes[1] & ((1 << 5) | (1 << 6))) >> 5);
    if (old_rf_link != keyboard->rf_link) {
        dprintf("rf link changed %02x\r\n", keyboard->rf_link);
    }

    return true;
}

// Periodic status supervisor. On every valid reply, re-fire
// rf_set_link_mode(commanded, 0) while the BK3632 reports neither connected
// nor paired, or reports a link mode other than the commanded one. That
// continuous kick snaps the chip's status reporting back to paired+connected
// after a power cycle where it re-links to the dongle on its own — without it
// the keyboard types fine but the status byte keeps saying unpaired and the
// indicator blinks as if pairing.
//
// Throttling: kb_update fires this every tick (~50 µs on Air60); body runs
// once every RF_SUPERVISOR_TICK_INTERVAL ticks. At interval 2000 the body
// runs ~10x per second.
#define RF_SUPERVISOR_TICK_INTERVAL 2000u
// Keep-alive suppression window after rf_set_link_pairing returns without
// pair-complete: the BK3632 is still advertising and a CMD_01 would abort
// the host's pairing handshake mid-flight. We keep polling so the indicator
// stays live and suppress just the keep-alive. At ~10 polls/s this is ~60 s,
// comfortably covering the BK3632's advertising window.
#define RF_PAIRING_WINDOW_POLLS 600u

// Link mode last commanded by us — the re-assert target. keyboard->rf_link
// can't serve here: it mirrors whatever mode the BK3632 *reports*, and the
// whole point is to correct the chip when the two disagree.
static __xdata uint8_t  commanded_link       = RF_MODE_2_4G;
static __xdata uint16_t pairing_window_polls = 0;

static __xdata uint16_t supervisor_ticks      = 0;
static __xdata uint8_t  supervisor_was_paired = 0;

void rf_link_supervisor(keyboard_state_t *keyboard)
{
    supervisor_ticks++;
    if (supervisor_ticks < RF_SUPERVISOR_TICK_INTERVAL) {
        return;
    }
    supervisor_ticks = 0;

    supervisor_was_paired = keyboard->paired;

    if (!rf_update_keyboard_state(keyboard)) {
        // No fresh frame — don't fire link commands off stale state.
        return;
    }

    // Pair-state edge detection. When the link transitions back to "paired",
    // re-assert the link mode (no pairing flag) so the BK3632 fully drops
    // its advertising state and switches into operational/notification mode.
    // Covers the case where pairing completes after rf_set_link_pairing's
    // burst window closed (its post-pair reassert handles the in-window
    // case).
    if (keyboard->paired && !supervisor_was_paired) {
        pairing_window_polls = 0;
        rf_reassert_link((rf_mode_t)commanded_link);
        return;
    }

    if (pairing_window_polls != 0) {
        pairing_window_polls--;
        return;
    }

    // Keep-alive: one CMD_01(commanded, 0) per valid poll while the link is
    // dead (neither connected nor paired) or the BK3632 reports the wrong mode.
    if ((!keyboard->connected && !keyboard->paired) || (keyboard->rf_link != commanded_link)) {
        rf_set_link_mode(commanded_link, 0);
    }
}

void rf_set_link(rf_mode_t link)
{
    commanded_link = (uint8_t)link;
    // Sent twice for reliability, with a brief gap.
    rf_set_link_mode(link, 0);
    delay_ms(20);
    rf_set_link_mode(link, 0);
}

void rf_apply_usb_mode(void)
{
    // Two sends with a brief delay, matching the rf_set_link dance.
    rf_cmd_06(1);
    delay_ms(20);
    rf_cmd_06(1);
}

static __xdata bool lazy_init_pending;

void rf_kbd_lazy_state_init(void)
{
    // Queue a one-shot "fresh baseline" send the next time
    // rf_send_kro_report runs (it zeroes the report on entry when this
    // flag is set).
    lazy_init_pending = true;
    // Resetting kro_prev_active = 0 means the next "all-zero baseline"
    // packet doesn't look like a release transition (prev=0, curr=0 →
    // no blanking queued), and cancelling any in-flight blanking
    // prevents leftover phantom/blank packets from a previous release
    // sequence colliding with the fresh baseline.
    kro_prev_active  = 0;
    blanking_pending = 0;
}

void rf_blanking_tick(void)
{
    if (blanking_pending == 0) {
        return;
    }

    // Pick phantom or blank based on the CURRENT counter value (before
    // decrement). pending == 6,4,2 → phantom (HID key 0x01 down).
    // pending == 5,3,1 → blank (all-zero release).
    //   buffer layout: [mods, key0, key1, key2, key3, key4]
    static __xdata uint8_t phantom_buf[6] = {0, 0x01, 0, 0, 0, 0};
    static __xdata uint8_t blank_buf[6]   = {0, 0, 0, 0, 0, 0};
    uint8_t               *buf            = ((blanking_pending & 1) == 0) ? phantom_buf : blank_buf;

    blanking_pending--;

    // Suppress re-scheduling while we drive the sequence — the phantom
    // packets are "active" and would otherwise trip the cancel path.
    blanking_active = true;
    rf_send_kro_report(buf);
    blanking_active = false;
}

// Module-static scratch for rf_set_link_pairing. Pushing the loop's locals
// to xdata so SDCC's OSEG packer doesn't run out of internal-RAM slots when
// rf_set_link_pairing inlines its callees.
static __xdata uint8_t pairing_status_bytes[2];
static __xdata uint8_t pairing_paired_now;

void rf_set_link_pairing(rf_mode_t link, __xdata keyboard_state_t *keyboard)
{
    commanded_link = (uint8_t)link;

    // Pairing trigger: send ONCE. Sending it twice the way rf_set_link does
    // is harmful here — the second rf_cmd_01(mode, 1) re-enters pairing mode
    // just as the host is mid-handshake, causing the BK3632 to abort the
    // connection.
    rf_set_link_mode(link, 1);

    // Post-pairing burst: ~100 ms to let the BK3632 enter advertising, then
    // up to 10 iterations of wake → 10 ms → status check.
    // Breaks as soon as `paired` flips so the indicator clears quickly and
    // the BK3632 isn't left waiting for status reads it expects.
    pairing_paired_now = 0;
    delay_ms(100);
    for (uint8_t tries = 10; tries > 0; tries--) {
        rf_wake_nudge();
        delay_ms(10);
        if (rf_get_status(pairing_status_bytes) && (pairing_status_bytes[0] & 0x80)) {
            keyboard->battery_level = pairing_status_bytes[0] & 0x07;
            keyboard->led_state     = pairing_status_bytes[1] & ((1 << 0) | (1 << 1) | (1 << 2));
            keyboard->connected     = (pairing_status_bytes[1] >> 3) & 1;
            keyboard->paired        = (pairing_status_bytes[1] >> 4) & 1;
            keyboard->low_power     = (pairing_status_bytes[1] >> 7) & 1;
            keyboard->rf_link       = ((pairing_status_bytes[1] & ((1 << 5) | (1 << 6))) >> 5);
            if (keyboard->paired) {
                pairing_paired_now = 1;
                break;
            }
        }
        delay_ms(20);
    }

    // Post-pair re-assertion. The pairing-flag variant (rf_cmd_01(mode, 1))
    // leaves the BK3632 in its "advertising/pairing" state machine — even
    // after a host has bonded, the BLE stack doesn't necessarily flip to
    // "operational, GATT notifications enabled" until it receives a follow-up
    // rf_cmd_01(mode, 0) (the non-pairing variant) telling it to drop pairing
    // mode and go fully operational on this link.
    //
    // This is the missing signal that otherwise prevents BT5 HID
    // notifications from reaching the host even though SMP bonding completes
    // cleanly. Fire it ONLY after pair-complete observed — firing too early
    // aborts mid-SMP.
    if (pairing_paired_now) {
        delay_ms(50);
        rf_reassert_link(link);
    } else {
        // Pairing didn't complete inside the burst — the BK3632 is still
        // advertising and the host may bond at any point. Hold the
        // supervisor's keep-alive off so it doesn't fire CMD_01 into the
        // handshake; the supervisor's paired-edge (or the window expiring)
        // closes it.
        pairing_window_polls = RF_PAIRING_WINDOW_POLLS;
    }
}

bool rf_get_status(uint8_t status_bytes[2])
{
    const uint8_t len = 4;

    rf_query_status();
    delay_us(100);
    rf_fetch_4();

    // Missing or corrupt reply: nudge the BK3632 awake with rf_wake_nudge so the
    // next status poll has a better chance of getting through.
    if (rf_tx_buf[0] != 0xBB) {
        rf_wake_nudge();
        return false;
    }

    uint8_t expected_sum = checksum(rf_tx_buf + 2, len - 2);
    if (rf_tx_buf[1] != expected_sum) {
        rf_wake_nudge();
        return false;
    }

    // rf_tx_buf[2] - 0x07 - 3 high power bits
    status_bytes[0] = rf_tx_buf[2];

    // rf_tx_buf[3] & (1 << 0)              - num lock (0 - off, 1 - on)
    // rf_tx_buf[3] & (1 << 1)              - caps lock (0 - off, 1 - on)
    // rf_tx_buf[3] & (1 << 2)              - scroll lock (0 - off, 1 - on)
    // rf_tx_buf[3] & (1 << 3)              - connection status (0 - disconnected, 1 - connected)
    // rf_tx_buf[3] & (1 << 4)              - pairing status (0 - not paired, 1 - paired)
    // rf_tx_buf[3] & ((1 << 5) | (1 << 6)) - rf mode (0 - 2.4G, 1 - BT1, 2 - BT2, 3 - BT3)
    // rf_tx_buf[3] & (1 << 7)              - low power bit
    status_bytes[1] = rf_tx_buf[3];

    return true;
}

void rf_fetch_4()
{
    rf_tx_buf[0] = 0xff;
    rf_tx_buf[1] = 0xff;
    rf_tx_buf[2] = 0xff;
    rf_tx_buf[3] = 0xff;

    // Receive variant: ~60 µs EA=0 around the byte loop so MISO sampling
    // has deterministic SCK timing. No ACK poll on the RX path.
    bb_spi_recv(rf_tx_buf, 4);
}

// Retry on no-ACK until the BK3632 acknowledges, looping in-place.
//
// DO NOT call rf_wake_nudge between attempts. rf_wake_nudge constructs
// its 6-byte CMD_0C packet into rf_tx_buf — the same buffer the caller
// is trying to send. If we wake-nudge mid-retry, the next attempt sends
// a buffer whose first 6 bytes are the wake-nudge prefix and the rest
// is the tail of the original report. The BK3632 sees a corrupted
// packet, won't forward the event to the host, and the key gets stuck
// on the host.
//
// Capped at RF_SEND_MAX_ATTEMPTS so a permanently-dead BK3632 doesn't
// brick the main loop. At ~400 µs per SPI burst the worst-case block
// is ~5 × 400 µs = 2 ms — well within main-loop tolerance.
#define RF_SEND_MAX_ATTEMPTS 5

static bool rf_send_or_retry(uint8_t *buf, int len)
{
    for (uint8_t attempt = 0; attempt < RF_SEND_MAX_ATTEMPTS; attempt++) {
        if (bb_spi_xfer(buf, len)) {
            return true;
        }
    }
    return false;
}

void rf_set_link_mode(uint8_t mode, uint8_t pairing)
{
    const uint8_t len = 6;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 3;
    rf_tx_buf[2] = 0x01;
    rf_tx_buf[3] = pairing;
    rf_tx_buf[4] = mode;

    rf_tx_buf[len - 1] = checksum(rf_tx_buf, len - 1);

    bb_spi_xfer(rf_tx_buf, 6);
}

// kro_prev_active tracks whether the previous CMD_REPORT carried any
// key activity, used only for release-transition detection in
// rf_kro_post_send_blanking — when this packet is idle (active == 0)
// and the previous was active, we just sent the first all-zero release
// packet and should queue the blanking tail.
//
// blanking_pending counts down from 6 to 0 after a release; each call
// to rf_blanking_tick sends one packet from the alternation:
//   pending == 6,4,2 (even, before decrement) → phantom (key 0x01)
//   pending == 5,3,1 (odd)                    → blank   (all-zero)
//
// The alternation fires 3 phantom packets (HID key 0x01 = ErrorRollOver)
// interleaved with 3 all-zero blanks. Because compute_byte9 is driven by
// current packet activity, phantom packets get byte9=0 and blank packets
// get byte9=1 — every packet has a unique signature on the wire so the
// BK3632 can't dedupe them.
//
// `blanking_active` suppresses re-scheduling while rf_blanking_tick is
// driving the sequence (the phantom packets are themselves "active"
// and would otherwise reset blanking_pending to 0 via the cancel path).
#define BLANKING_COUNT_AFTER_RELEASE 6

static void rf_kro_post_send_blanking(bool curr_active)
{
    if (blanking_active) {
        // We're inside rf_blanking_tick. The tick owns the counter.
        return;
    }
    if (curr_active) {
        // New (or continued) press cancels any pending blanking; the
        // active packet is sufficient on its own.
        blanking_pending = 0;
    } else if (kro_prev_active) {
        // First all-zero packet after a release transition. Queue the
        // phantom/blank tail.
        blanking_pending = BLANKING_COUNT_AFTER_RELEASE;
    }
}

bool rf_send_kro_report(uint8_t *buffer)
{
    const uint8_t len = 32;

    // If lazy init is pending, override the caller's buffer with an
    // all-zero baseline for this packet — the BK3632 receives a clean
    // release and any keys still held get re-detected by the next matrix
    // sweep.
    static __xdata uint8_t empty_buf[6] = {0, 0, 0, 0, 0, 0};
    if (lazy_init_pending) {
        lazy_init_pending = false;
        buffer            = empty_buf;
    }

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 3;
    rf_tx_buf[2] = CMD_REPORT;
    rf_tx_buf[3] = buffer[0];
    rf_tx_buf[4] = buffer[1];
    rf_tx_buf[5] = buffer[2];
    rf_tx_buf[6] = buffer[3];
    rf_tx_buf[7] = buffer[4];
    rf_tx_buf[8] = buffer[5];

    uint8_t active = 0;
    for (__xdata int i = 0; i < 6; i++) {
        if (buffer[i] != 0) {
            active = 1;
            break;
        }
    }
    rf_tx_buf[9] = compute_byte9(active);

    // rf_tx_buf[10..25] carries the NKRO bit state even for CMD_02 (6KRO)
    // packets. We don't maintain a parallel NKRO buffer when running 6KRO,
    // so synthesize it from the 5 key bytes in `buffer`: each HID keycode
    // K maps to bit (K & 7) of byte (10 + (K >> 3)).
    for (int i = 10; i < 31; i++) {
        rf_tx_buf[i] = 0x00;
    }
    for (int j = 1; j <= 5; j++) {
        uint8_t kc = buffer[j];
        if (kc != 0 && (kc >> 3) < 16) {
            rf_tx_buf[10 + (kc >> 3)] |= (uint8_t)(1u << (kc & 7));
        }
    }

    rf_tx_buf[31] = checksum(rf_tx_buf, len - 1);

    const bool ack = rf_send_or_retry(rf_tx_buf, len);

    rf_kro_post_send_blanking(active);
    kro_prev_active = active;
    return ack;
}

void rf_send_nkro_report(__xdata uint8_t mods, __xdata uint8_t *nkro_buffer)
{
    const uint8_t len = 32;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 3;
    rf_tx_buf[2] = CMD_REPORT;
    rf_tx_buf[3] = mods;

    rf_tx_buf[4] = 0x00;
    rf_tx_buf[5] = 0x00;
    rf_tx_buf[6] = 0x00;
    rf_tx_buf[7] = 0x00;
    rf_tx_buf[8] = 0x00;

    uint8_t active = (mods != 0);
    if (!active) {
        for (__xdata int i = 0; i < 20; i++) {
            if (nkro_buffer[i] != 0) {
                active = 1;
                break;
            }
        }
    }
    rf_tx_buf[9] = compute_byte9(active);

    rf_tx_buf[10] = nkro_buffer[0];
    rf_tx_buf[11] = nkro_buffer[1];
    rf_tx_buf[12] = nkro_buffer[2];
    rf_tx_buf[13] = nkro_buffer[3];
    rf_tx_buf[14] = nkro_buffer[4];
    rf_tx_buf[15] = nkro_buffer[5];
    rf_tx_buf[16] = nkro_buffer[6];
    rf_tx_buf[17] = nkro_buffer[7];
    rf_tx_buf[18] = nkro_buffer[8];
    rf_tx_buf[19] = nkro_buffer[9];
    rf_tx_buf[20] = nkro_buffer[10];
    rf_tx_buf[21] = nkro_buffer[11];
    rf_tx_buf[22] = nkro_buffer[12];
    rf_tx_buf[23] = nkro_buffer[13];
    rf_tx_buf[24] = nkro_buffer[14];
    rf_tx_buf[25] = nkro_buffer[15];
    rf_tx_buf[26] = nkro_buffer[16];
    rf_tx_buf[27] = nkro_buffer[17];
    rf_tx_buf[28] = nkro_buffer[18];
    rf_tx_buf[29] = nkro_buffer[19];

    rf_tx_buf[30] = 0x00;

    rf_tx_buf[31] = checksum(rf_tx_buf, len - 1);

    rf_send_or_retry(rf_tx_buf, len);

    rf_kro_post_send_blanking(active);
    kro_prev_active = active;
}

void rf_cmd_03(uint8_t param)
{
    const uint8_t len = 6;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 3;
    rf_tx_buf[2] = 0x03;
    rf_tx_buf[3] = param;
    rf_tx_buf[4] = 0x00;

    rf_tx_buf[len - 1] = checksum(rf_tx_buf, len - 1);

    bb_spi_xfer(rf_tx_buf, 6);
}

void rf_cmd_04()
{
    const uint8_t len = 4;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 3;
    rf_tx_buf[2] = 0x04;

    rf_tx_buf[len - 1] = checksum(rf_tx_buf, 3);

    bb_spi_xfer(rf_tx_buf, 4);
}

void rf_send_consumer_system(uint16_t consumer, uint16_t system)
{
    const uint8_t len = 14;

    rf_tx_buf[0]  = MAGIC_BYTE;
    rf_tx_buf[1]  = len - 3;
    rf_tx_buf[2]  = 0x05;
    rf_tx_buf[3]  = 0x00;
    rf_tx_buf[4]  = 0x00;
    rf_tx_buf[5]  = 0x00;
    rf_tx_buf[6]  = 0x00;
    rf_tx_buf[7]  = 0x00;
    rf_tx_buf[8]  = 0x00;
    rf_tx_buf[9]  = consumer & 0xff;
    rf_tx_buf[10] = consumer >> 8;
    rf_tx_buf[11] = system & 0xff;
    rf_tx_buf[12] = system >> 8;

    rf_tx_buf[len - 1] = checksum(rf_tx_buf, len - 1);

    rf_send_or_retry(rf_tx_buf, len);
}

void rf_cmd_06(uint8_t param) // 0x00 or 0x01
{
    const uint8_t len = 6;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 3;
    rf_tx_buf[2] = 0x06;
    rf_tx_buf[3] = param;
    rf_tx_buf[4] = 0x00;

    rf_tx_buf[len - 1] = checksum(rf_tx_buf, len - 1);

    bb_spi_xfer(rf_tx_buf, len);
}

void rf_prepare_sleep(uint8_t param)
{
    const uint8_t len = 6;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 3;
    rf_tx_buf[2] = 0x07;
    rf_tx_buf[3] = param;
    rf_tx_buf[4] = 0x00;

    rf_tx_buf[len - 1] = checksum(rf_tx_buf, len - 1);

    bb_spi_xfer(rf_tx_buf, len);
}

void rf_set_bt_name(uint8_t type, char *name)
{
    const uint8_t len = 32;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 3;
    rf_tx_buf[2] = 0x08;
    rf_tx_buf[3] = type;

    // [4] = name length, [5..5+name_len-1] = name chars.
    uint8_t name_len = 0;
    int     i        = 5;
    while (*name && i < (len - 1)) {
        rf_tx_buf[i] = *name;
        name++;
        i++;
        name_len++;
    }
    rf_tx_buf[4] = name_len;

    for (int j = i; j < (len - 1); j++) {
        rf_tx_buf[j] = 0x00;
    }

    rf_tx_buf[len - 1] = checksum(rf_tx_buf, len - 1);

    bb_spi_xfer(rf_tx_buf, len);
    // Wait 20 ms before firing rf_cmd_04 (the COMMIT). < 20 ms is sometimes
    // not enough for the BK3632 to finish parsing the name buffer, and the
    // commit then applies to an incomplete slot.
    delay_ms(20);
    rf_cmd_04();
}

void rf_query_status()
{
    const uint8_t len = 6;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 3;
    rf_tx_buf[2] = 0x0a;
    rf_tx_buf[3] = 0x00;
    rf_tx_buf[4] = 0x00;

    rf_tx_buf[len - 1] = checksum(rf_tx_buf, len - 1);

    bb_spi_xfer(rf_tx_buf, len);
}

void rf_wake_from_sleep()
{
    const uint8_t len = 6;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 3;
    rf_tx_buf[2] = 0x0b;
    rf_tx_buf[3] = 0x00;
    rf_tx_buf[4] = 0x00;

    rf_tx_buf[len - 1] = checksum(rf_tx_buf, len - 1);

    bb_spi_xfer(rf_tx_buf, len);
}

void rf_wake_nudge()
{
    const uint8_t len = 6;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 3;
    rf_tx_buf[2] = 0x0c;
    rf_tx_buf[3] = 0x00;
    rf_tx_buf[4] = 0x00;

    rf_tx_buf[len - 1] = checksum(rf_tx_buf, len - 1);

    bb_spi_xfer(rf_tx_buf, len);
}

uint8_t checksum(uint8_t *data, int len)
{
    uint8_t checksum = 0;

    for (int i = 0; i < len; i++) {
        checksum += data[i];
    }

    return 0x55 - checksum;
}
