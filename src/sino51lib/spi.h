#pragma once

#include <stdbool.h>
#include <stdint.h>

// Hardware SPI master. The pins are fixed by the part, not chosen by the board: see its
// spi_hw.h. For a bus on arbitrary pins use bb_spi instead.
//
// The board sets the clock and frame format in kbdef.h before including anything that
// reaches here: KB_SPI_CLOCK_DIV (an fSYS divider, a power of two the part supports),
// KB_SPI_CPOL, KB_SPI_CPHA and KB_SPI_LSB_FIRST.

void spi_init(void);
void spi_deinit(void);

// Full duplex: the byte goes out, the byte the slave shifted back comes in.
uint8_t spi_xfer_byte(uint8_t out);

// Same, over a buffer, replacing each byte with what came back.
void spi_xfer(uint8_t *data, uint8_t len);

void spi_send(const uint8_t *data, uint8_t len);
void spi_recv(uint8_t *data, uint8_t len, uint8_t idle);
