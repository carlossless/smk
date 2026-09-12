#pragma once

#include "sh68f89.h"

// RSTSTAT is 0xB1 on SFR page 0 and USBCON on page 1, so a kick taken with page 1 latched
// writes 0x02 into USBCON and drops the D+ pull-up. This, and delay_us()/delay_ms() which
// kick from inside their loop, are page 0 only.
#define WATCHDOG_PERIOD (uint8_t)(_WDT1)

#ifdef WATCHDOG_ENABLE
#    define watchdog_kick() (RSTSTAT = WATCHDOG_PERIOD)
#else
#    define watchdog_kick() ((void)0)
#endif
