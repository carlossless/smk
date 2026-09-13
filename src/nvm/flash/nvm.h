#pragma once

#include "nvm_hw.h"
#include <stdbool.h>
#include <stdint.h>

#define NVM_CAPACITY (NVM_SECTOR_SIZE - 4u)

bool nvm_load(__xdata uint8_t *dst, uint8_t len);

void nvm_save(const __xdata uint8_t *src, uint8_t len);
