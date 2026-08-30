#pragma once

#include "sh68f881.h"

#define WATCHDOG_PERIOD 2

#ifdef WATCHDOG_ENABLE
#    define watchdog_kick() (RSTSTAT = WATCHDOG_PERIOD)
#else
#    define watchdog_kick() ((void)0)
#endif
