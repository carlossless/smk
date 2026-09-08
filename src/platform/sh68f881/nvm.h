#pragma once

#include <stdint.h>
#include <stdbool.h>

// persists a small record in the on-chip EEPROM-like data block; it survives power cycles and, unlike the program flash, a reflash.

#define NVM_SECTOR_SIZE 256u
#define NVM_CAPACITY    (NVM_SECTOR_SIZE - 4u)

bool nvm_load(__xdata uint8_t *dst, uint8_t len);

void nvm_save(const __xdata uint8_t *src, uint8_t len);
