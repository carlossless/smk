#include "rf_controller.h"
#include <stdint.h>
#include "delay.h"
#include "debug.h"
#include "bb_spi.h" // FIXME: should be conditional?

#define MAGIC_BYTE 0xaa
#define CMD_REPORT 0x02

// typedef enum {
//     RF_MODE_2_4G = 0x00,
//     RF_MODE_BT1  = 0x01,
//     RF_MODE_BT2  = 0x02,
//     RF_MODE_BT3  = 0x03
// } rf_mode_t;

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

void    rf_send_blank_report();
bool    rf_get_status(uint8_t status_bytes[2]);
void    rf_cmd_01(uint8_t mode, uint8_t pairing);
void    rf_cmd_02(uint8_t *buffer);
void    rf_cmd_02_nkro(__xdata uint8_t mods, __xdata uint8_t *nkro_buffer);
void    rf_cmd_03(uint8_t param);
void    rf_cmd_04();
void    rf_cmd_05(uint16_t consumer, uint16_t system);
void    rf_cmd_06(uint8_t param);
void    rf_cmd_07(uint8_t param);
void    rf_cmd_08(uint8_t type, char *name);
void    rf_cmd_0a();
void    rf_cmd_0b();
void    rf_cmd_0c();
void    rf_fetch_4();
uint8_t checksum(uint8_t *data, int len);

void rf_init()
{
    __xdata uint8_t status_bytes[2];

    rf_cmd_0c();
    delay_ms(6);
    rf_get_status(status_bytes);
    delay_us(200);
    rf_cmd_08(RF_SET_NAME_BT5, rf_bt5_name);
    delay_ms(12);
    rf_cmd_04();
    delay_ms(32);
    rf_cmd_08(RF_SET_NAME_BT3, rf_bt3_name);
    delay_ms(12);
    rf_cmd_04();
    delay_ms(30);
    rf_cmd_01(RF_MODE_2_4G, RF_PAIRING_OFF);
    delay_ms(20);
    rf_cmd_01(RF_MODE_2_4G, RF_PAIRING_OFF);
    delay_ms(15);
    rf_cmd_05(0, 0);
    delay_ms(30);
    rf_send_blank_report();
}

__xdata uint8_t kro6buffer[6];

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

    if (blank) {
        // blanking sequence
        kro6buffer[1] = 0x00;
        rf_cmd_02(kro6buffer);
        kro6buffer[1] = 0x01;
        rf_cmd_02(kro6buffer);
        kro6buffer[1] = 0x00;
        rf_cmd_02(kro6buffer);
        kro6buffer[1] = 0x01;
        rf_cmd_02(kro6buffer);
        kro6buffer[1] = 0x00;
        rf_cmd_02(kro6buffer);
        kro6buffer[1] = 0x01;
        rf_cmd_02(kro6buffer);
        kro6buffer[1] = 0x00;
        rf_cmd_02(kro6buffer);
    } else {
        rf_cmd_02(kro6buffer);
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
        // blanking sequence
        kro6buffer[1] = 0x00;
        rf_cmd_02(kro6buffer);
        kro6buffer[1] = 0x01;
        rf_cmd_02(kro6buffer);
        kro6buffer[1] = 0x00;
        rf_cmd_02(kro6buffer);
        kro6buffer[1] = 0x01;
        rf_cmd_02(kro6buffer);
        kro6buffer[1] = 0x00;
        rf_cmd_02(kro6buffer);
        kro6buffer[1] = 0x01;
        rf_cmd_02(kro6buffer);
        kro6buffer[1] = 0x00;
        rf_cmd_02(kro6buffer);
    } else {
        rf_cmd_02_nkro(report->mods, report->bits);
    }
}

void rf_send_extra(__xdata report_extra_t *report)
{
    switch (report->report_id) {
        case REPORT_ID_SYSTEM:
            rf_cmd_05(0, report->usage);
            break;
        case REPORT_ID_CONSUMER:
            rf_cmd_05(report->usage, 0);
            break;
    }
}

void rf_update_keyboard_state(keyboard_state_t *keyboard)
{
    __xdata uint8_t status_bytes[2];
    rf_get_status(status_bytes);

    keyboard->led_state = status_bytes[1] & ((1 << 0) | (1 << 1) | (1 << 2)); // num, caps, scroll

    // keyboard->conn_status = status_bytes[1] & (1 << 3);
    // keyboard->pairing_status = status_bytes[1] & (1 << 4);
    uint8_t old_rf_link = keyboard->rf_link;
    keyboard->rf_link   = ((status_bytes[1] & ((1 << 5) | (1 << 6))) >> 5);
    if (old_rf_link != keyboard->rf_link) {
        dprintf("rf link changed: %d\r\n", keyboard->rf_link);
    }
}

void rf_set_link(rf_mode_t link)
{
    rf_cmd_01(link, 0);
}

void rf_send_blank_report()
{
    const uint8_t len = 32;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 3;
    rf_tx_buf[2] = CMD_REPORT;
    for (int i = 3; i < 31; i++) {
        rf_tx_buf[i] = 0x00;
    }

    rf_tx_buf[31] = checksum(rf_tx_buf, len - 1);

    bb_spi_xfer(rf_tx_buf, len);
}

bool rf_get_status(uint8_t status_bytes[2])
{
    const uint8_t len = 4;

    rf_cmd_0a();
    delay_us(100);
    rf_fetch_4();

    if (rf_tx_buf[0] != 0xBB) {
        return false;
    }

    uint8_t expected_sum = checksum(rf_tx_buf + 2, len - 2);
    if (rf_tx_buf[1] != expected_sum) {
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

    bb_spi_xfer(rf_tx_buf, 4);
}

void rf_cmd_01(uint8_t mode, uint8_t pairing)
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

void rf_cmd_02(uint8_t *buffer)
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

    bb_spi_xfer(rf_tx_buf, len);
}

void rf_cmd_02_nkro(__xdata uint8_t mods, __xdata uint8_t *nkro_buffer)
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

    bb_spi_xfer(rf_tx_buf, len);
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

void rf_cmd_05(uint16_t consumer, uint16_t system)
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

    bb_spi_xfer(rf_tx_buf, len);
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

void rf_cmd_07(uint8_t param)
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

void rf_cmd_08(uint8_t type, char *name)
{
    const uint8_t len = 32;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 3;
    rf_tx_buf[2] = 0x08;
    rf_tx_buf[3] = type;
    int i        = 4;
    while (*name) {
        rf_tx_buf[4] = *name;
        name++;
        i++;
    }
    for (int j = i; j < (len - 1); j++) {
        rf_tx_buf[j] = 0x00;
    }

    rf_tx_buf[len - 1] = checksum(rf_tx_buf, len - 1);

    bb_spi_xfer(rf_tx_buf, len);
}

void rf_cmd_0a()
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

void rf_cmd_0b()
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

void rf_cmd_0c()
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
