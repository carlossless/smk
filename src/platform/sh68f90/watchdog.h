#pragma once

#include "sh68f90.h"

#define WATCHDOG_PERIOD 0

#ifdef WATCHDOG_ENABLE
#    define watchdog_kick() (RSTSTAT = WATCHDOG_PERIOD)
#else
#    define watchdog_kick() ((void)0)
#endif
