#pragma once

#include "gpio.h"

// The bit-banged master takes its two pins from here, as five macros rather than a pin number,
// because a board may have to select an SFR page or invert a level to reach them. P3.6 and
// P3.7 are a placeholder; both need an external pull-up, which is what RELEASE relies on.

#define BB_SDA 0x80u // P3.7
#define BB_SCL 0x40u // P3.6

#define KB_I2C_SDA_LOW()        \
    do {                        \
        GPIO_LOW(3, BB_SDA);    \
        GPIO_OUTPUT(3, BB_SDA); \
    } while (0)
#define KB_I2C_SDA_RELEASE() GPIO_INPUT(3, BB_SDA)
#define KB_I2C_SDA_READ()    ((P3 & BB_SDA) != 0)

#define KB_I2C_SCL_LOW()        \
    do {                        \
        GPIO_LOW(3, BB_SCL);    \
        GPIO_OUTPUT(3, BB_SCL); \
    } while (0)
#define KB_I2C_SCL_RELEASE() GPIO_INPUT(3, BB_SCL)

// nops per half bit, so roughly 100 kHz at 24 MHz
#define KB_I2C_HALF_PERIOD 10u
