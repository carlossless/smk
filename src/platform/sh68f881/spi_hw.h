#pragma once

#include "sfr.h"

#define SPI_HW_PRESENT 1

// MOSI P2.4, MISO P2.5, SCK P2.6, SS P2.7. SPEN takes the pins over; their port registers
// need no setup. SPR saturates at 128, so 101 through 111 all divide by 128.
#define SPI_DIV_MIN 4u
#define SPI_DIV_MAX 128u
