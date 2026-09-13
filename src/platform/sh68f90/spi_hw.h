#pragma once

#include "sfr.h"

#define SPI_HW_PRESENT 1

// SCK P7.4, SS P7.3, MOSI P7.2, MISO P7.1, or P3.3/P3.4/P3.5 with MAPPING set to 0xa5.
// SPEN takes the pins over; their port registers need no setup.
#define SPI_DIV_MIN 4u
#define SPI_DIV_MAX 512u
