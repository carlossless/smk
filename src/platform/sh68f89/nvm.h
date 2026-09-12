#pragma once

#include <stdint.h>
#include <stdbool.h>

// persists a small record in the board's external 24Cxx EEPROM; the on-chip EEPROM-like
// block is left alone. Boards supply the bus pins through kbdef.h.

#define NVM_RECORD_SIZE 32u
#define NVM_CAPACITY    (NVM_RECORD_SIZE - 4u)

// true when the settings EEPROM acknowledges its device address.
bool nvm_present(void);

bool nvm_load(__xdata uint8_t *dst, uint8_t len);

void nvm_save(const __xdata uint8_t *src, uint8_t len);
