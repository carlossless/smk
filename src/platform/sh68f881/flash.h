#pragma once

#include <stdint.h>
#include <stdbool.h>

// Generic persistence of a small record (up to ~500 bytes) in the reserved on-chip
// flash sector, via the SH68F881 Sector Self-Programming (SSP) feature. Survives power
// cycles, but a firmware reflash wipes it. See flash.c for the storage layout & safety.

bool flash_settings_load(__xdata uint8_t *dst, uint8_t len);

void flash_settings_save(const __xdata uint8_t *src, uint8_t len);
