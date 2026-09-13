#pragma once

#include "flash.h"

// The EEPROM-like block is 2 KB of 256-byte sectors from zero in the FAC window, and unlike a
// program-flash sector it survives a reflash. Boards here keep settings off-chip, so nothing
// builds nvm.c yet; this is what it would use.
#define NVM_WINDOW      FLASH_DATA
#define NVM_BASE        0u
#define NVM_SECTOR_SIZE 256u
