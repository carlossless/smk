#include "rf_controller.h"
#include <stdint.h>
#include "bb_spi.h" // FIXME: should be conditional?

#define MAGIC_BYTE 0xaa
#define CMD_REPORT 0x02

__xdata uint8_t rf_tx_buf[32];

void rf_send_blank_report();
// void rf_cmd_generic(uint8_t cmd, uint8_t *buf, uint8_t len);
void rf_cmd_01(uint8_t param1, uint8_t param2);
void rf_cmd_03(uint8_t param);
void rf_cmd_04();
void rf_cmd_06(uint8_t param);
void rf_cmd_08(uint8_t param1, uint8_t param2);
void rf_cmd_0a();
void rf_cmd_0c();
void rf_fetch_4();
uint8_t checksum(uint8_t *data, int len);

void rf_init()
{
    rf_cmd_0c();
    rf_keep_alive();
    rf_cmd_08(0x00, 0x35);
    rf_cmd_04();
    rf_cmd_08(0x01, 0x33);
    rf_cmd_04();
    rf_cmd_01(0x00, 0x00);
    rf_cmd_01(0x00, 0x00);
    rf_send_blank_report();
}

void rf_send_report(report_keyboard_t *report)
{
    const uint8_t len = 32;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 3;
    rf_tx_buf[2] = CMD_REPORT;
    rf_tx_buf[3] = report->raw[0];
    rf_tx_buf[4] = report->raw[2];
    rf_tx_buf[5] = report->raw[3];
    rf_tx_buf[6] = report->raw[4];
    rf_tx_buf[7] = report->raw[5];
    rf_tx_buf[8] = report->raw[6];
    // FIXME: last keyboard report key is lost
    rf_tx_buf[9] = 0x00; // 0x00 or 0x01

    for (int i = 10; i < 31; i++) { // FIXME: NKRO bytes are blanked out until NKRO is implemented
        rf_tx_buf[i] = 0x00;
    }

    rf_tx_buf[31] = checksum(rf_tx_buf, len - 1);

    bb_spi_xfer(rf_tx_buf, len);
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

void rf_keep_alive()
{
    rf_cmd_0a();
    rf_fetch_4();
}

void rf_fetch_4()
{
    rf_tx_buf[0] = 0xff;
    rf_tx_buf[1] = 0xff;
    rf_tx_buf[2] = 0xff;
    rf_tx_buf[3] = 0xff;

    bb_spi_xfer(rf_tx_buf, 4);
}

void rf_cmd_01(uint8_t param1, uint8_t param2) // ??, 0x00 or 0x01
{
    const uint8_t len = 6;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 3;
    rf_tx_buf[2] = 0x01;
    rf_tx_buf[3] = param1;
    rf_tx_buf[4] = param2;
    rf_tx_buf[len - 1] = checksum(rf_tx_buf, len - 1);

    bb_spi_xfer(rf_tx_buf, 6);
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
    rf_tx_buf[3] = checksum(rf_tx_buf, 3);

    bb_spi_xfer(rf_tx_buf, 4);
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

void rf_cmd_08(uint8_t param1, uint8_t param2) // 0x00 0x01, 0x35 0x33
{
    const uint8_t len = 32;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 3;
    rf_tx_buf[2] = 0x08;
    rf_tx_buf[3] = param1;
    rf_tx_buf[4] = 0x0b;
    rf_tx_buf[5] = 0x41;
    rf_tx_buf[6] = 0x69;
    rf_tx_buf[7] = 0x72;
    rf_tx_buf[8] = 0x36;
    rf_tx_buf[9] = 0x30;
    rf_tx_buf[10] = 0x20;
    rf_tx_buf[11] = 0x42;
    rf_tx_buf[12] = 0x54;
    rf_tx_buf[13] = param2;
    rf_tx_buf[14] = 0x2e;
    rf_tx_buf[15] = 0x30;
    rf_tx_buf[16] = 0x00;
    rf_tx_buf[17] = 0x00;
    rf_tx_buf[18] = 0x00;
    rf_tx_buf[19] = 0x00;
    rf_tx_buf[20] = 0x00;
    rf_tx_buf[21] = 0x00;
    rf_tx_buf[22] = 0x00;
    rf_tx_buf[23] = 0x00;
    rf_tx_buf[24] = 0x00;
    rf_tx_buf[25] = 0x00;
    rf_tx_buf[26] = 0x00;
    rf_tx_buf[27] = 0x00;
    rf_tx_buf[28] = 0x00;
    rf_tx_buf[29] = 0x00;
    rf_tx_buf[30] = 0x00;
    rf_tx_buf[len - 1] = checksum(rf_tx_buf, len - 1);

    bb_spi_xfer(rf_tx_buf, len);
}

void rf_cmd_0a()
{
    const uint8_t len = 6;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 1;
    rf_tx_buf[2] = 0x0a;
    rf_tx_buf[3] = 0x00;
    rf_tx_buf[4] = 0x00;
    rf_tx_buf[len - 1] = checksum(rf_tx_buf, len - 1);

    bb_spi_xfer(rf_tx_buf, len);
}

void rf_cmd_0c()
{
    const uint8_t len = 6;

    rf_tx_buf[0] = MAGIC_BYTE;
    rf_tx_buf[1] = len - 1;
    rf_tx_buf[2] = 0x0c;
    rf_tx_buf[3] = 0x00;
    rf_tx_buf[4] = 0x00;
    rf_tx_buf[len - 1] = checksum(rf_tx_buf, len - 1);

    bb_spi_xfer(rf_tx_buf, len);
}

// void rf_cmd_generic(uint8_t cmd, uint8_t *buf, uint8_t len)
// {
//     uint8_t total_len = len + 3;
//     rf_tx_buf[0] = MAGIC_BYTE;
//     rf_tx_buf[1] = len;
//     int i;
//     for (i = 0; i < len; i++) {
//         rf_tx_buf[i + 2] = buf[i];
//     }
//     rf_tx_buf[i + 2] = checksum(rf_tx_buf, total_len);

//     bb_spi_xfer(rf_tx_buf, total_len);
// }

uint8_t checksum(uint8_t *data, int len)
{
    uint8_t checksum = 0;

    for (int i = 0; i < len; i++) {
        checksum += data[i];
    }

    return 0x55 - checksum;
}
