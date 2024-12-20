#pragma once

#include <stdio.h>

#define dprintf(...)                    \
    do {                                \
        if (DEBUG) printf(__VA_ARGS__); \
    } while (0)
