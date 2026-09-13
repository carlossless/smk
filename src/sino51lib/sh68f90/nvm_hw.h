#pragma once

#include "flash.h"

#define NVM_WINDOW      FLASH_CODE
#define NVM_BASE        FLASH_CFG_ADDR
#define NVM_SECTOR_SIZE FLASH_CFG_SIZE

_Static_assert((NVM_BASE & (NVM_SECTOR_SIZE - 1)) == 0, "the store must start on a flash sector boundary");
_Static_assert(NVM_BASE + NVM_SECTOR_SIZE <= FLASH_MARKER_ADDR, "the store must stay clear of the boot marker sector");
