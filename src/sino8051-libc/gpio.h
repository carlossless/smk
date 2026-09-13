#pragma once

#include "sfr.h"

#define GPIO_OUTPUT(port, mask)         \
    do {                                \
        P##port##CR |= (uint8_t)(mask); \
    } while (0)
#define GPIO_INPUT(port, mask)           \
    do {                                 \
        P##port##CR &= (uint8_t)~(mask); \
    } while (0)
#define GPIO_DIR_WRITE(port, value)     \
    do {                                \
        P##port##CR = (uint8_t)(value); \
    } while (0)

#define GPIO_PULLUP_ON(port, mask)       \
    do {                                 \
        P##port##PCR |= (uint8_t)(mask); \
    } while (0)
#define GPIO_PULLUP_OFF(port, mask)       \
    do {                                  \
        P##port##PCR &= (uint8_t)~(mask); \
    } while (0)
#define GPIO_PULLUP_WRITE(port, value)   \
    do {                                 \
        P##port##PCR = (uint8_t)(value); \
    } while (0)

#define GPIO_HIGH(port, mask)       \
    do {                            \
        P##port |= (uint8_t)(mask); \
    } while (0)
#define GPIO_LOW(port, mask)         \
    do {                             \
        P##port &= (uint8_t)~(mask); \
    } while (0)
#define GPIO_WRITE(port, value)     \
    do {                            \
        P##port = (uint8_t)(value); \
    } while (0)
