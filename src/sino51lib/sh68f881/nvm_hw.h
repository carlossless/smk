#pragma once

#include "flash.h"

// The EEPROM-like block is 2 KB of 256-byte sectors from zero in the FAC window, and unlike a
// program-flash sector it survives a reflash. The record lives in the first sector.
#define NVM_WINDOW      FLASH_DATA
#define NVM_BASE        0u
#define NVM_SECTOR_SIZE 256u
