#pragma once

#include "sfr.h"

#ifdef WATCHDOG_ENABLE
#    define watchdog_kick() (RSTSTAT = WATCHDOG_PERIOD)
#else
#    define watchdog_kick() ((void)0)
#endif
