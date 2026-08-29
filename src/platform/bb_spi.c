#include "bb_spi.h"
#include "kbdef.h"
#include "delay.h"

#define _nop_() __asm nop __endasm

uint8_t bb_spi_xfer_byte(uint8_t data);

// After CS rises, poll RF_BB_SPI_ACK for the BK3632 to toggle the line
// (~150 µs total).
#define RF_BB_SPI_ACK_POLL_MAX 50
#define RF_BB_SPI_ACK_POLL_US  3

// Open-drain SPI: SCK/MOSI/CS/MOT idle as INPUT with pull-up (high). To
// drive LOW, we briefly switch the pin's PxCR direction bit to OUTPUT while
// the latch holds 0. To release HIGH, we switch back to INPUT and let the
// pull-up take it.
#define MOT_DRIVE_LOW()    \
    do {                   \
        RF_BB_SPI_MOT = 0; \
        P0CR |= _P0_5;     \
        RF_BB_SPI_MOT = 0; \
    } while (0)
#define MOT_RELEASE_HIGH()       \
    do {                         \
        P0CR &= (uint8_t)~_P0_5; \
        RF_BB_SPI_MOT = 1;       \
    } while (0)
#define CS_DRIVE_LOW()    \
    do {                  \
        RF_BB_SPI_CS = 0; \
        P7CR |= _P7_4;    \
        RF_BB_SPI_CS = 0; \
    } while (0)
#define CS_RELEASE_HIGH()        \
    do {                         \
        P7CR &= (uint8_t)~_P7_4; \
        RF_BB_SPI_CS = 1;        \
    } while (0)

// Sequence shared between send and receive: drive MOT low, 3 µs wakeup,
// drive CS low, bit-bang `len` bytes, release CS/MOSI/MOT high. The
// byte loop runs under __critical iff `lock`.
static void bb_spi_burst(uint8_t *data, int len, bool lock)
{
    MOT_DRIVE_LOW();
    delay_us(3); // wakeup
    CS_DRIVE_LOW();
    if (lock) {
        __critical
        {
            for (int i = 0; i < len; i++) {
                data[i] = bb_spi_xfer_byte(data[i]);
            }
        }
    } else {
        for (int i = 0; i < len; i++) {
            data[i] = bb_spi_xfer_byte(data[i]);
        }
    }
    CS_RELEASE_HIGH();
    // Release MOSI to input (pull-up high) after the last bit.
    P0CR &= (uint8_t)~_P0_7;
    RF_BB_SPI_MOSI = 1;
    MOT_RELEASE_HIGH();
}

bool bb_spi_xfer(uint8_t *data, int len)
{
    bool ack_initial = RF_BB_SPI_ACK;

    bb_spi_burst(data, len, false);

    for (uint8_t tries = 0; tries < RF_BB_SPI_ACK_POLL_MAX; tries++) {
        if (RF_BB_SPI_ACK != ack_initial) {
            return true;
        }
        delay_us(RF_BB_SPI_ACK_POLL_US);
    }

    return false;
}

void bb_spi_recv(uint8_t *data, int len)
{
    bb_spi_burst(data, len, true); // no ACK poll on the RX path
}

uint8_t bb_spi_xfer_byte(uint8_t data)
{
    uint8_t recv = 0;

    // Per-bit open-drain bit-bang: SCK low, set MOSI, sample MISO, release SCK
    // high. MISO is sampled while SCK is LOW - the slave latches it on the SCK
    // falling edge, so the master must read before releasing SCK high again.
    for (uint8_t i = 0; i < 8; i++) {
        recv = recv << 1;

        RF_BB_SPI_SCK = 0;
        P4CR |= _P4_7;
        RF_BB_SPI_SCK = 0;

        if (data & (1 << 7)) {
            P0CR &= (uint8_t)~_P0_7; // MOSI -> input, pull-up to high
            RF_BB_SPI_MOSI = 1;
        } else {
            RF_BB_SPI_MOSI = 0;
            P0CR |= _P0_7; // MOSI -> output, drives low
            RF_BB_SPI_MOSI = 0;
        }

        if (RF_BB_SPI_MISO) {
            recv |= 0x01;
        }

        P4CR &= (uint8_t)~_P4_7;
        RF_BB_SPI_SCK = 1;

        data = data << 1;
    }

    return recv;
}
