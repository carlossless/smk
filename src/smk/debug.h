#pragma once

#include "console.h"

#if DEBUG == 1
#    define dprintf(...) console_printf(__VA_ARGS__)
#else
#    define dprintf(...) ((void)0)
#endif

// Use instead of an `if` around dprintf: an empty branch trips --Werror.
#if DEBUG == 1
#    define dprintf_if(cond, ...)           \
        do {                                \
            if (cond) dprintf(__VA_ARGS__); \
        } while (0)
#else
#    define dprintf_if(cond, ...) ((void)(cond))
#endif
