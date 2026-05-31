#include "rf_controller.h"
#include <stdint.h>
#include "delay.h"
#include "debug.h"
#include "bb_spi.h" // FIXME: should be conditional?
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

__code const char *rf_bt5_name = "SMK BT5.0";
__code const char *rf_bt3_name = "SMK BT3.0";

__xdata uint8_t rf_tx_buf[32];

bool    rf_get_status(uint8_t status_bytes[2]);
void    rf_set_link_mode(uint8_t mode, uint8_t pairing);
void    rf_send_kro_report(uint8_t *buffer);
void    rf_send_nkro_report(__xdata uint8_t mods, __xdata uint8_t *nkro_buffer);
// Pairing / reconnect handshake step. Stock fires this with arg 2 then arg 3
// in sequence during the factory-reset / link-reconnect path; exact semantics
// of the arg are unknown.
void    rf_cmd_03(uint8_t param);
// Commit/apply settings. Stock only sends this immediately after a
// rf_set_bt_name (it's chained inline there), so this looks like a "finalise
// the pending config" signal to the BK3632.
void    rf_cmd_04();
void    rf_send_consumer_system(uint16_t consumer, uint16_t system);
// USB-mode signal. Stock sends this with arg=1 from its mode-detection path
// and idle tick when the keyboard is in wired mode. Exact protocol unclear.
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

    // Register both BT names. Timings mirror stock's rf_cmd_08 caller:
    //   SET_BT5 -> 50 ms -> SET_BT3 -> 5 ms
    // The 50 ms gap is what gives the BK3632 enough time to fully commit
    // the BT5 name before the BT3 packet arrives. With < ~50 ms here, the
    // BT3 commit silently fails and the chip falls back to its default
    // name ("Air60 BT3.0"). rf_set_bt_name internally does the 20 ms
    // SET -> COMMIT delay that stock also does.
    rf_set_bt_name(RF_SET_NAME_BT5, rf_bt5_name);
    delay_ms(50);
    rf_set_bt_name(RF_SET_NAME_BT3, rf_bt3_name);
    delay_ms(5);

    // Stock fires rf_cmd_01 (or rf_cmd_06) within milliseconds of the BT3
    // commit, via mode_detect. Empirically the BK3632 seems to need that
    // follow-up SPI packet to actually persist the BT3 name slot — without
    // it the second commit silently drops. main() later calls rf_set_link
    // again with the saved link from flash; this default call here is just
    // to flush the BT3 commit.
    rf_set_link(RF_MODE_2_4G);
    // Link mode is applied separately by rf_set_link() once user settings
    // have been loaded from flash, so we boot back into whatever mode was
    // last selected (BT1/2/3 or 2.4G).
}

__xdata uint8_t kro6buffer[6];

// All-keys-released "blanking" state machine. On a blank report we send the
// user's release once, then arm the state machine to emit alternating
// (key1=1, all-zeros) packets one-per-tick for 3 cycles (= 6 follow-up
// packets). Spreading them across main-loop ticks lets the PWM ISR scan in
// between, so the LED indicators don't flicker.
static __xdata uint8_t rf_blanking_active  = 0;
static __xdata uint8_t rf_blanking_phase   = 0; // 0 = next packet is key1=1, 1 = next is all-zeros
static __xdata uint8_t rf_blanking_counter = 0; // counts pairs sent; stops after 3

// Throttle: rf_blanking_tick() is called every kb_update iteration, but
// emits a packet only every Nth invocation. ~1ms between packets is plenty
// for the LED scan to refresh.
#define RF_BLANKING_TICK_THROTTLE 200
static __xdata uint16_t rf_blanking_throttle = 0;

void rf_send_report(__xdata report_keyboard_t *report)
{
    kro6buffer[0] = report->raw[0];
    kro6buffer[1] = report->raw[2];
    kro6buffer[2] = report->raw[3];
    kro6buffer[3] = report->raw[4];
    kro6buffer[4] = report->raw[5];
    kro6buffer[5] = report->raw[6];

    bool blank = true;
    for (__xdata int i = 0; i < 6; i++) {
        if (kro6buffer[i] != 0) {
            blank = false;
        }
    }

    rf_send_kro_report(kro6buffer);

    if (blank) {
        // Arm the blanking state machine. The follow-up packets are emitted
        // from rf_blanking_tick() on subsequent main-loop iterations.
        rf_blanking_active   = 1;
        rf_blanking_phase    = 0;
        rf_blanking_counter  = 0;
        rf_blanking_throttle = 0;
    } else {
        // New key activity cancels any pending blanking.
        rf_blanking_active = 0;
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

    dprintf("is blank: %d\r\n", blank); // FIXME: a delay is necessary here

    if (blank) {
        // NKRO release uses the same 6-byte cmd-02 buffer + blanking sequence
        // as the regular KRO release.
        for (__xdata int i = 0; i < 6; i++) {
            kro6buffer[i] = 0;
        }
        rf_send_kro_report(kro6buffer);

        rf_blanking_active   = 1;
        rf_blanking_phase    = 0;
        rf_blanking_counter  = 0;
        rf_blanking_throttle = 0;
    } else {
        rf_send_nkro_report(report->mods, report->bits);
        rf_blanking_active = 0;
    }
}

void rf_blanking_tick(void)
{
    if (!rf_blanking_active) {
        return;
    }

    if (++rf_blanking_throttle < RF_BLANKING_TICK_THROTTLE) {
        return;
    }
    rf_blanking_throttle = 0;

    if (rf_blanking_phase == 0) {
        // First-half packet: key1 = 0x01, rest = 0.
        kro6buffer[0] = 0;
        kro6buffer[1] = 1;
        kro6buffer[2] = 0;
        kro6buffer[3] = 0;
        kro6buffer[4] = 0;
        kro6buffer[5] = 0;
        rf_send_kro_report(kro6buffer);
        rf_blanking_phase = 1;
    } else {
        // Second-half packet: all zeros.
        for (__xdata int i = 0; i < 6; i++) {
            kro6buffer[i] = 0;
        }
        rf_send_kro_report(kro6buffer);
        rf_blanking_phase = 0;
        rf_blanking_counter++;
        if (rf_blanking_counter > 2) {
            // 3 alternating cycles is enough for the BK3632 to register the
            // release.
            rf_blanking_active  = 0;
            rf_blanking_counter = 0;
        }
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

void rf_update_keyboard_state(keyboard_state_t *keyboard)
{
    __xdata uint8_t status_bytes[2];

    // Single-shot status query. rf_query_status (send) runs with EA enabled;
    // rf_fetch_4 brackets just its receive burst with EA=0. The LED scan
    // therefore keeps running through the send and only pauses for the
    // ~600 µs receive — invisible.
    //
    // If the BK3632 was asleep / didn't ack (status_bytes[0] bit 7 not set,
    // or rf_get_status returned false on bad magic / checksum), we just skip
    // this poll. rf_get_status sends rf_wake_nudge on its way out so the next
    // poll cycle starts with the BK3632 already nudged awake.
    bool ok = rf_get_status(status_bytes);

    if (!ok || !(status_bytes[0] & 0x80)) {
        return; // no fresh status — leave keyboard_state untouched
    }

    // status_bytes[0]: bits 0-2 = battery level (0..7), bit 7 = ack flag (already checked)
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
        dprintf("rf link changed: %d\r\n", keyboard->rf_link);
    }
}

void rf_set_link(rf_mode_t link)
{
    // Sent twice for reliability, with a brief gap — stock does the same in
    // its mode-application path.
    rf_set_link_mode(link, 0);
    delay_ms(20);
    rf_set_link_mode(link, 0);
}

void rf_set_link_pairing(rf_mode_t link, __xdata keyboard_state_t *keyboard)
{
    __xdata uint8_t status_bytes[2];

    // Pairing trigger: send ONCE. Stock's pairing path also sends once
    // (it only retries the cmd on SPI no-ack, up to 10x). Sending it
    // twice the way rf_set_link does is harmful here — the second
    // rf_cmd_01(mode, 1) re-enters pairing mode just as the host is
    // mid-handshake, causing the BK3632 to abort the connection.
    rf_set_link_mode(link, 1);

    // Mirror stock's post-pairing burst: ~100 ms to let the BK3632 enter
    // advertising, then up to 10 iterations of wake → 10 ms → status check.
    // Breaks as soon as `paired` flips so the indicator clears quickly and
    // the BK3632 isn't left waiting for status reads it expects. Parse the
    // status inline here instead of going through rf_update_keyboard_state
    // so the call chain stays shallow enough for SDCC's OSEG packer.
    delay_ms(100);
    for (uint8_t tries = 10; tries > 0; tries--) {
        rf_wake_nudge();
        delay_ms(10);
        if (rf_get_status(status_bytes) && (status_bytes[0] & 0x80)) {
            keyboard->battery_level = status_bytes[0] & 0x07;
            keyboard->led_state     = status_bytes[1] & ((1 << 0) | (1 << 1) | (1 << 2));
            keyboard->connected     = (status_bytes[1] >> 3) & 1;
            keyboard->paired        = (status_bytes[1] >> 4) & 1;
            keyboard->low_power     = (status_bytes[1] >> 7) & 1;
            keyboard->rf_link       = ((status_bytes[1] & ((1 << 5) | (1 << 6))) >> 5);
            if (keyboard->paired) {
                break;
            }
        }
        delay_ms(20);
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

    // Disable interrupts for the receive burst so the 4-byte response struct
    // can't be partially parsed by an ISR sneaking in between bytes. Send-
    // only paths leave EA on.
    EA = 0;
    bb_spi_xfer(rf_tx_buf, 4);
    EA = 1;
}

// Send a packet; if the BK3632 doesn't ack within bb_spi_xfer's polling
// window, fire a wake-nudge and retry the packet once.
static bool rf_send_or_retry(uint8_t *buf, int len)
{
    if (bb_spi_xfer(buf, len)) {
        return true;
    }
    rf_wake_nudge();
    delay_us(200);
    return bb_spi_xfer(buf, len);
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

void rf_send_kro_report(uint8_t *buffer)
{
    const uint8_t len = 32;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 3;
    rf_tx_buf[2] = CMD_REPORT;
    rf_tx_buf[3] = buffer[0];
    rf_tx_buf[4] = buffer[1];
    rf_tx_buf[5] = buffer[2];
    rf_tx_buf[6] = buffer[3];
    rf_tx_buf[7] = buffer[4];
    rf_tx_buf[8] = buffer[5];
    rf_tx_buf[9] = 0x00; // 0x00 or 0x01

    for (int i = 10; i < 31; i++) { // FIXME: NKRO bytes are blanked out until they are implemented
        rf_tx_buf[i] = 0x00;
    }

    rf_tx_buf[31] = checksum(rf_tx_buf, len - 1);

    rf_send_or_retry(rf_tx_buf, len);
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

    rf_tx_buf[9] = 0x00; // 0x00 or 0x01

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
}

void rf_cmd_03(uint8_t param) // ?? or 0x02
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

    // [4] = name length, [5..5+name_len-1] = name chars. Matches stock's
    // packet layout, which also stores the length at offset 4 and the
    // string starting at offset 5.
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
    // Stock waits exactly 20 ms here before firing rf_cmd_04 (the COMMIT).
    // < 20 ms is sometimes not enough for the BK3632 to finish parsing the
    // name buffer, and the commit then applies to an incomplete slot.
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
