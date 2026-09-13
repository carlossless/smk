#pragma once

#include "sfr.h"

#define SPI_HW_PRESENT 1

// SCK P0.0, SS P0.1, MOSI P0.2, MISO P0.3. SPEN takes the pins over; their port registers
// need no setup.
#define SPI_DIV_MIN 2u
#define SPI_DIV_MAX 256u
