#pragma once

#include <stdio.h>

// Tried routing through `printf_small` to save DSEG. Net loss: printf_small
// is marked _REENTRANT and parks its locals in ISEG instead, ballooning
// ISEG from 6 to 44 bytes and pushing SSEG (stack) *down* by ~14 bytes.
// `printf_large` is paradoxically better for our memory model because its
// non-reentrant statics overlay into DSEG/OSEG (which SDCC then folds via
// the call-graph), and ISEG stays small. Sticking with default printf.

#define dprintf(...)                    \
    do {                                \
        if (DEBUG) printf(__VA_ARGS__); \
    } while (0)
