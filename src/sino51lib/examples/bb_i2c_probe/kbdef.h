#pragma once

#include "gpio.h"

#define BB_SDA 0x80u
#define BB_SCL 0x40u

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

#define KB_I2C_HALF_PERIOD 10u
