#include "bb_spi.h"
#include "kbdef.h"

uint8_t bb_spi_xfer_byte(uint8_t data);

void bb_spi_xfer(uint8_t *data, int len)
{
    RF_BB_SPI_CS = 0;
    for (int i = 0; i < len; i++) {
        data[i] = bb_spi_xfer_byte(data[i]);
    }
    RF_BB_SPI_CS = 1;
}

uint8_t bb_spi_xfer_byte(uint8_t data)
{
    uint8_t recv = 0;

    for (uint8_t i = 0; i < 8; i++) {
        RF_BB_SPI_SCK = 0;

        if (data & 0x01) {
            RF_BB_SPI_MOSI = 1;
        } else {
            RF_BB_SPI_MOSI = 0;
        }

        recv |= RF_BB_SPI_MISO ? 1 : 0;

        RF_BB_SPI_SCK = 1;

        data = data >> 1;
        recv = recv << 1;
    }

    return recv;
}
