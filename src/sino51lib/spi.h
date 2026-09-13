#pragma once

#include <stdbool.h>
#include <stdint.h>

void spi_init(void);
void spi_deinit(void);

uint8_t spi_xfer_byte(uint8_t out);

void spi_xfer(uint8_t *data, uint8_t len);

void spi_send(const uint8_t *data, uint8_t len);
void spi_recv(uint8_t *data, uint8_t len, uint8_t idle);
