#pragma once

#include <stdbool.h>
#include <stdint.h>

void twi_init(void);
void twi_deinit(void);

bool twi_start(void);
void twi_stop(void);

bool    twi_write_address(uint8_t addr7, bool read);
bool    twi_write(uint8_t value);
uint8_t twi_read(bool ack);
