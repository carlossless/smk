#pragma once

#include <stdbool.h>
#include <stdint.h>

bool bb_spi_xfer(uint8_t *data, int len);

void bb_spi_recv(uint8_t *data, int len);
