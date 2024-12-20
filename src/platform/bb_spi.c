#include "bb_spi.h"
#include "kbdef.h"
#include "delay.h"

#define _nop_() __asm nop __endasm

uint8_t bb_spi_xfer_byte(uint8_t data);

void bb_spi_xfer(uint8_t *data, int len)
{
    EA            = 0;
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
    EA             = 1;
}

uint8_t bb_spi_xfer_byte(uint8_t data)
{
    uint8_t recv = 0;

    for (uint8_t i = 0; i < 8; i++) {
        recv = recv << 1;

        RF_BB_SPI_SCK = 0;

        // TODO: probably not necessary
        // trying to keep the clock speed consistent
        _nop_();
        _nop_();
        _nop_();
        _nop_();

        _nop_();
        _nop_();
        _nop_();
        _nop_();

        if (data & (1 << 7)) {
            RF_BB_SPI_MOSI = 1;
        } else {
            RF_BB_SPI_MOSI = 0;
        }

        if (RF_BB_SPI_MISO) {
            recv |= 0x01;
        }

        RF_BB_SPI_SCK = 1;

        _nop_();
        _nop_();
        _nop_();
        _nop_();

        data = data << 1;
    }

    return recv;
}
