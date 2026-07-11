#pragma once

#include "console.h"

// Routes to our own tiny console_printf(), NOT SDCC's printf. Neither
// printf_large (link-fails: OSEG overflow in internal RAM) nor printf_small
// (_REENTRANT, balloons ISEG 6->44 bytes) fits this firmware's internal-RAM
// ceiling; see project memory. console_printf keeps its scratch in __xdata.
// Subset only: %% %c %s %d %u %x %X with an optional '0' flag + 1 width digit.

#define dprintf(...)                            \
    do {                                        \
        if (DEBUG) console_printf(__VA_ARGS__); \
    } while (0)
