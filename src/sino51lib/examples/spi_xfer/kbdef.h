#pragma once

// What a board would set. The divider has to be one this part's SPR ladder can reach, which
// spi.c checks at compile time against spi_hw.h.
#define KB_SPI_CLOCK_DIV 16u
#define KB_SPI_CPOL      0u
#define KB_SPI_CPHA      0u
#define KB_SPI_LSB_FIRST 0u
