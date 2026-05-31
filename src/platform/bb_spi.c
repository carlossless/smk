#include "bb_spi.h"
#include "kbdef.h"
#include "delay.h"

#define _nop_() __asm nop __endasm

uint8_t bb_spi_xfer_byte(uint8_t data);

// After CS rises we poll RF_BB_SPI_ACK for up to RF_BB_SPI_ACK_POLL_MAX
// iterations of RF_BB_SPI_ACK_POLL_US µs each, watching for the BK3632 to
// toggle the line. ~150 µs total.
#define RF_BB_SPI_ACK_POLL_MAX 50
#define RF_BB_SPI_ACK_POLL_US  3

bool bb_spi_xfer(uint8_t *data, int len)
{
    // No EA toggle: ISRs that fire mid-burst can lengthen SCK low/high but
    // cannot corrupt the bus, because every ISR that writes P0/P4/P7 uses
    // masked ANL/ORL or single-bit SETB/CLR that preserves the SPI pin
    // bits. Lets the LED PWM scan keep running through the burst.
    bool ack_initial = RF_BB_SPI_ACK;

    RF_BB_SPI_MOT = 0;
    // wakeup?
    delay_us(3);
    RF_BB_SPI_CS = 0;
    for (int i = 0; i < len; i++) {
        data[i] = bb_spi_xfer_byte(data[i]);
    }
    RF_BB_SPI_CS   = 1;
    RF_BB_SPI_MOSI = 1;
    RF_BB_SPI_MOT  = 1;

    for (uint8_t tries = 0; tries < RF_BB_SPI_ACK_POLL_MAX; tries++) {
        if (RF_BB_SPI_ACK != ack_initial) {
            return true;
        }
        delay_us(RF_BB_SPI_ACK_POLL_US);
    }

    return false;
}

uint8_t bb_spi_xfer_byte(uint8_t data)
{
    uint8_t recv = 0;

    // No padding NOPs: the natural instruction overhead between SCK
    // transitions already gives the BK3632 enough setup/hold time. Per bit
    // is ~23 cycles, giving roughly a 1 MHz SCK on a 24 MHz oscillator.
    for (uint8_t i = 0; i < 8; i++) {
        recv          = recv << 1;
        RF_BB_SPI_SCK = 0;

        if (data & (1 << 7)) {
            RF_BB_SPI_MOSI = 1;
        } else {
            RF_BB_SPI_MOSI = 0;
        }

        if (RF_BB_SPI_MISO) {
            recv |= 0x01;
        }

        RF_BB_SPI_SCK = 1;
        data          = data << 1;
    }

    return recv;
}
