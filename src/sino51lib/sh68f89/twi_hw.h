#pragma once

#include "sfr.h"

#define TWI_HW_PRESENT 1

// SCL P6.6, SDA P6.7. The block is on SFR page 1, where 0xb1 is USBCON and not RSTSTAT, so
// the driver has to drop back to page 0 before it can kick the watchdog.
#define TWI_SFR_PAGE_1 1
