#ifndef UTILS_H
#define UTILS_H

// clang-format off

#define _nop_() \
    __asm     \
    nop   \
    __endasm

#define _nop_3_() \
    __asm     \
    nop   \
    nop   \
    nop   \
    __endasm

#define _nop_4_() \
    __asm     \
    nop   \
    nop   \
    nop   \
    nop   \
    __endasm

// clang-format on

/**
 * A macro that returns the statically known size of the array.
 */
#define ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))

#define STRINGIFY(x) #x
#define TOSTRING(x)  STRINGIFY(x)

#endif
