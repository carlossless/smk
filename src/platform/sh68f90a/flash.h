#pragma once

#include <stdint.h>
#include <stdbool.h>

// Generic persistence of a small record (up to ~500 bytes) in the reserved on-chip
// flash sector, via the SH68F90A Sector Self-Programming (SSP) feature. Survives power
// cycles, but a firmware reflash wipes it. See flash.c for the storage layout & safety.

// Copies the stored record into `dst` and returns true iff a valid record of exactly
// `len` bytes exists; otherwise leaves `dst` untouched and returns false.
bool flash_settings_load(__xdata uint8_t *dst, uint8_t len);

// Erase + rewrite the record (no-op if the stored bytes already match `src`). Briefly
// disables interrupts while the flash operation runs.
void flash_settings_save(const __xdata uint8_t *src, uint8_t len);
