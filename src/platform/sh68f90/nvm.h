#pragma once

#include <stdint.h>
#include <stdbool.h>

#define NVM_CAPACITY (FLASH_CFG_SIZE - 4u)

bool nvm_load(__xdata uint8_t *dst, uint8_t len);

void nvm_save(const __xdata uint8_t *src, uint8_t len);
