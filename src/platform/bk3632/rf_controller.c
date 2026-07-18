#include "rf_controller.h"
#include <stdint.h>
#include "delay.h"
#include "debug.h"
#include "bb_spi.h" // FIXME: should be conditional?
#include "sh68f90a.h"

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
// `__code const char *` would put the pointer itself in DSEG, eating iRAM.
static const __code char rf_bt5_name[] = "SMK BT5.0";
static const __code char rf_bt3_name[] = "SMK BT3.0";

__xdata uint8_t rf_tx_buf[32];

static __xdata uint8_t kro_prev_active;
static __xdata uint8_t blanking_pending;
static __xdata bool    blanking_active;

// byte9 override flags. mac_mode_compat forces byte9 to 0 ("active"); macOS
// needs every packet flagged active, else the BK3632 dedupes idle packets and
// the host misses releases. byte9_disable skips only the idle branch.
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
void rf_send_nkro_report(uint8_t mods, __xdata uint8_t *nkro_buffer);
// BT state-management command (internal — public callers go through
// rf_factory_reset_bonds). arg=2 wipes all BT bonds and must be followed by
// rf_init to reload the BT names the wipe clears.
void rf_cmd_03(uint8_t param);
// Commit the pending config. Only meaningful immediately after rf_set_bt_name.
void rf_cmd_04();
void rf_send_consumer_system(uint16_t consumer, uint16_t system);
// USB-mode signal (arg=1 when the keyboard is in wired mode).
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
    uint8_t status_bytes[2];

    // The BK3632 takes ~1.5 s after VCC stabilizes to be ready for SPI;
    // commands sent before then are dropped.
    delay_ms(255);
    delay_ms(255);
    delay_ms(255);
    delay_ms(255);
    delay_ms(255);
    delay_ms(255);

    // Up to 10 wake-nudge + status-query attempts, decreasing delay each pass.
    // Break when status byte 0 reports the ack flag set.
    for (uint8_t tries = 10; tries > 0; tries--) {
        rf_wake_nudge();
        delay_ms(tries);
        if (rf_get_status(status_bytes) && (status_bytes[0] & 0x80)) {
            break;
        }
    }

    // Register both BT names. The 50 ms gap lets the BK3632 fully commit the
    // BT5 name before the BT3 packet arrives; with < ~50 ms the BT3 commit
    // silently fails and the chip falls back to its default name.
    rf_set_bt_name(RF_SET_NAME_BT5, rf_bt5_name);
    delay_ms(50);
    rf_set_bt_name(RF_SET_NAME_BT3, rf_bt3_name);
    delay_ms(5);

    // A follow-up SPI packet within milliseconds of the BT3 commit is needed to
    // persist the BT3 name slot — without it the commit silently drops. main()
    // re-calls rf_set_link with the saved link, so this just flushes the commit.
    rf_set_link(RF_MODE_2_4G);
}

// Fire rf_set_link_mode(mode, 0) twice with a small gap. The double-fire locks
// the BK3632 into the requested link slot — a single shot is sometimes dropped
// if the chip is mid-state-transition (advertising → connected → bonded).
void rf_reassert_link(rf_mode_t link)
{
    rf_set_link_mode((uint8_t)link, 0);
    delay_ms(20);
    rf_set_link_mode((uint8_t)link, 0);
}

// Wipe all BT bonds / factory reset. Destructive: tears down BLE advertising
// and committed BT names, so rf_init must re-run afterwards. Recovers a BK3632
// that's accumulated bad BLE state across repeated failed pairings (rotating-MAC
// advertising the host can't pair against because SMP keeps timing out).
void rf_factory_reset_bonds(void)
{
    rf_cmd_03(2); // wipe stored bonds
    delay_ms(200);
    // Sleep/wake forces the BK3632 to re-run its state machine from scratch.
    rf_prepare_sleep(0);
    delay_ms(100);
    rf_wake_from_sleep();
    delay_ms(200);
    rf_init(); // reload BT names cleared by the wipe + sleep cycle
    delay_ms(200);
}

__xdata uint8_t kro6buffer[6];

// Send queue. rf_send_report stashes the 6KRO snapshot here and sends
// immediately; if the send fails (unresponsive after RF_SEND_MAX_ATTEMPTS),
// rf_pending stays set and rf_send_pending_flush retries from the same buffer.
// Each new matrix event overwrites the buffer, so retries converge to current.
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
    // Leave rf_pending set on no-ACK so kb_update's next call re-enters.
    if (rf_send_kro_report(rf_pending_buf)) {
        rf_pending = false;
    }
}

void rf_send_nkro(__xdata report_nkro_t *report)
{
    bool blank = true;
    for (int i = 1; i < NKRO_REPORT_SIZE - 1; i++) {
        if (report->raw[i] != 0) {
            blank = false;
        }
    }

    if (blank) {
        // NKRO release: re-use the 6-byte cmd-02 buffer with all keys zero.
        for (int i = 0; i < 6; i++) {
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
    uint8_t status_bytes[2];

    if (!rf_get_status(status_bytes)) {
        return false; // no fresh status — leave keyboard_state untouched
    }

    // Bit 7 is the "awake" marker. A frame is valid on magic + checksum alone
    // and is still applied when this bit is clear; the bit only triggers a
    // wake-nudge. Discarding such frames would freeze the displayed link state
    // (e.g. paired=0 captured mid-reconnect) while the keyboard kept working.
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
// nor paired, or a link mode other than commanded. This snaps the chip's
// status reporting back to paired+connected after a power cycle where it
// re-links on its own but keeps reporting unpaired (indicator blinks as if
// pairing). Body runs once every RF_SUPERVISOR_TICK_INTERVAL ticks (~10x/s).
#define RF_SUPERVISOR_TICK_INTERVAL 2000u
// Keep-alive suppression window after rf_set_link_pairing returns without
// pair-complete: the BK3632 is still advertising and a CMD_01 would abort the
// host's pairing handshake. Keep polling (indicator stays live), suppress just
// the keep-alive. At ~10 polls/s this is ~60 s, covering the advertising window.
#define RF_PAIRING_WINDOW_POLLS 600u

// Link mode last commanded by us — the re-assert target. keyboard->rf_link
// can't serve here: it mirrors what the BK3632 *reports*, and the point is to
// correct the chip when the two disagree.
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

    // Pair-state edge: on the transition back to "paired", re-assert the link
    // (no pairing flag) so the BK3632 drops advertising and goes operational.
    // Covers pairing completing after rf_set_link_pairing's burst window closed.
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
    // Two sends with a brief delay for reliability.
    rf_cmd_06(1);
    delay_ms(20);
    rf_cmd_06(1);
}

static __xdata bool lazy_init_pending;

void rf_kbd_lazy_state_init(void)
{
    // Queue a one-shot fresh-baseline send (rf_send_kro_report zeroes the
    // report on entry when this flag is set).
    lazy_init_pending = true;
    // prev=0 so the baseline packet doesn't look like a release transition,
    // and cancel any in-flight blanking so leftover packets from a prior
    // release don't collide with the fresh baseline.
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

    // Suppress re-scheduling while we drive the sequence — the phantom packets
    // are "active" and would otherwise trip the cancel path.
    blanking_active = true;
    rf_send_kro_report(buf);
    blanking_active = false;
}

// Module-static scratch for rf_set_link_pairing — keeps the loop's locals out
// of internal RAM so SDCC's OSEG packer doesn't run out of slots.
static __xdata uint8_t pairing_status_bytes[2];
static __xdata uint8_t pairing_paired_now;

void rf_set_link_pairing(rf_mode_t link, __xdata keyboard_state_t *keyboard)
{
    commanded_link = (uint8_t)link;

    // Pairing trigger: send ONCE. A second rf_cmd_01(mode, 1) re-enters pairing
    // mode mid-handshake and makes the BK3632 abort the connection.
    rf_set_link_mode(link, 1);

    // Post-pairing burst: ~100 ms to let the BK3632 enter advertising, then up
    // to 10 iterations of wake → 10 ms → status check. Breaks as soon as
    // `paired` flips so the indicator clears quickly.
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

    // Post-pair re-assertion. After a host bonds, the BK3632's BLE stack won't
    // flip to operational (GATT notifications enabled) until it receives a
    // non-pairing rf_cmd_01(mode, 0). Without this, BT5 HID notifications never
    // reach the host even though SMP bonding completes cleanly. Fire it ONLY
    // after pair-complete — firing too early aborts mid-SMP.
    if (pairing_paired_now) {
        delay_ms(50);
        rf_reassert_link(link);
    } else {
        // Pairing didn't complete in the burst — the BK3632 is still advertising
        // and the host may bond later. Hold off the supervisor's keep-alive so
        // it doesn't fire CMD_01 into the handshake; the paired-edge (or the
        // window expiring) closes it.
        pairing_window_polls = RF_PAIRING_WINDOW_POLLS;
    }
}

bool rf_get_status(uint8_t status_bytes[2])
{
    const uint8_t len = 4;

    rf_query_status();
    delay_us(100);
    rf_fetch_4();

    // Missing or corrupt reply: nudge the BK3632 awake so the next poll has a
    // better chance of getting through.
    if (rf_tx_buf[0] != 0xBB) {
        rf_wake_nudge();
        return false;
    }

    uint8_t expected_sum = checksum(rf_tx_buf + 2, len - 2);
    if (rf_tx_buf[1] != expected_sum) {
        rf_wake_nudge();
        return false;
    }

    // [2]: bits 0-2 = battery level, bit 7 = awake marker
    status_bytes[0] = rf_tx_buf[2];
    // [3]: bit 0 = num lock, bit 1 = caps lock, bit 2 = scroll lock,
    //      bit 3 = connected, bit 4 = paired, bits 5-6 = rf mode
    //      (0=2.4G, 1=BT1, 2=BT2, 3=BT3), bit 7 = low power
    status_bytes[1] = rf_tx_buf[3];

    return true;
}

void rf_fetch_4()
{
    rf_tx_buf[0] = 0xff;
    rf_tx_buf[1] = 0xff;
    rf_tx_buf[2] = 0xff;
    rf_tx_buf[3] = 0xff;

    bb_spi_recv(rf_tx_buf, 4);
}

// Retry on no-ACK, looping in-place. DO NOT call rf_wake_nudge between
// attempts: it builds its packet into rf_tx_buf — the same buffer being sent —
// so a mid-retry nudge corrupts the report, the BK3632 drops it, and the key
// gets stuck on the host. Capped at RF_SEND_MAX_ATTEMPTS (~2 ms worst case) so
// a dead BK3632 doesn't brick the main loop.
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

// After a release, blanking_pending counts down from 6; rf_blanking_tick sends
// one packet per step, alternating 3 phantom packets (HID key 0x01,
// ErrorRollOver) with 3 all-zero blanks. Since compute_byte9 tracks activity,
// phantoms get byte9=0 and blanks byte9=1 — a unique wire signature per packet
// so the BK3632 can't dedupe them and drop the release.
#define BLANKING_COUNT_AFTER_RELEASE 6

static void rf_kro_post_send_blanking(bool curr_active)
{
    if (blanking_active) {
        return; // inside rf_blanking_tick; the tick owns the counter
    }
    if (curr_active) {
        blanking_pending = 0; // a press cancels any pending blanking
    } else if (kro_prev_active) {
        // First all-zero packet after a release — queue the phantom/blank tail.
        blanking_pending = BLANKING_COUNT_AFTER_RELEASE;
    }
}

bool rf_send_kro_report(uint8_t *buffer)
{
    const uint8_t len = 32;

    // On pending lazy init, override the buffer with an all-zero baseline so
    // the BK3632 gets a clean release; held keys re-detect on the next sweep.
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
    for (int i = 0; i < 6; i++) {
        if (buffer[i] != 0) {
            active = 1;
            break;
        }
    }
    rf_tx_buf[9] = compute_byte9(active);

    // Leave the NKRO bitmap region zero: mirroring 6KRO keys here makes the dongle emit a modifier-less duplicate that breaks chords like cmd+space.
    for (int i = 10; i < 31; i++) {
        rf_tx_buf[i] = 0x00;
    }

    rf_tx_buf[31] = checksum(rf_tx_buf, len - 1);

    const bool ack = rf_send_or_retry(rf_tx_buf, len);

    rf_kro_post_send_blanking(active);
    kro_prev_active = active;
    return ack;
}

void rf_send_nkro_report(uint8_t mods, __xdata uint8_t *nkro_buffer)
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
        for (int i = 0; i < 20; i++) {
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

    // [4] = name length, [5..] = name chars.
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
    // 20 ms before the COMMIT: with less, the BK3632 may not finish parsing the
    // name buffer and the commit applies to an incomplete slot.
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
