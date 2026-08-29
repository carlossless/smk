#pragma once

#include "sh68f90a.h"

// Writing RSTSTAT restarts the watchdog interval; the value selects the period,
// and 0 is the longest one (1024 ms).
#define WATCHDOG_PERIOD 0

#ifdef WATCHDOG_ENABLE
#    define watchdog_kick() (RSTSTAT = WATCHDOG_PERIOD)
#else
#    define watchdog_kick() ((void)0)
#endif
