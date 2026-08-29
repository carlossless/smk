#pragma once

#include "console.h"

// Debug-only printf, compiled out entirely outside DEBUG builds (console_printf
// only exists there). Format subset: %% %c %s %d %u %x %X, with an optional '0'
// flag and one width digit.

#if DEBUG == 1
#    define dprintf(...) console_printf(__VA_ARGS__)
#else
#    define dprintf(...) ((void)0)
#endif

// Log only when `cond` holds. Use this rather than wrapping dprintf in a plain
// `if`: outside DEBUG builds that leaves a conditional with an empty body, which
// SDCC folds away and reports as "conditional flow changed by optimizer" -
// a --Werror build failure. Here the condition is discarded instead, so any
// variable that exists only to feed it still counts as used.
#if DEBUG == 1
#    define dprintf_if(cond, ...)           \
        do {                                \
            if (cond) dprintf(__VA_ARGS__); \
        } while (0)
#else
#    define dprintf_if(cond, ...) ((void)(cond))
#endif
