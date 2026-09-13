#pragma once

#include "nvm_hw.h"
#include <stdbool.h>
#include <stdint.h>

// Persists one record in whichever on-chip store the part offers: a reserved program-flash
// sector, or the EEPROM-like data block where there is one. The board picks neither; see the
// part's nvm_hw.h. A board with an off-chip store uses that driver instead of this one, and
// the two are alternatives, never both.
//
// Layout: magic, magic, length, payload, checksum. Four bytes of that are not payload.

#define NVM_CAPACITY (NVM_SECTOR_SIZE - 4u)

bool nvm_load(__xdata uint8_t *dst, uint8_t len);

void nvm_save(const __xdata uint8_t *src, uint8_t len);
