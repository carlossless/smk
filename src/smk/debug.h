#pragma once

#include "console.h"

// Debug-only printf: no-op unless DEBUG. Routes to console_printf() — subset
// only: %% %c %s %d %u %x %X with an optional '0' flag + 1 width digit.

#define dprintf(...)                            \
    do {                                        \
        if (DEBUG) console_printf(__VA_ARGS__); \
    } while (0)
