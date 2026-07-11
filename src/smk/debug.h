#pragma once

#include "console.h"

#define dprintf(...)                            \
    do {                                        \
        if (DEBUG) console_printf(__VA_ARGS__); \
    } while (0)
