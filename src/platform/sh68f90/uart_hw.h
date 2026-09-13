#pragma once

#include "sfr.h"

#define UART_HW_PRESENT 1

#define UART_VECTOR  _INT_EUART0
#define UART_IEN     IEN1
#define UART_IEN_BIT _ES0

#define UART_SBRT_WRITE(sbrt)                                 \
    do {                                                      \
        SBRTH = (uint8_t)(((uint16_t)(sbrt) >> 8) | _SBRTEN); \
        SBRTL = (uint8_t)(sbrt);                              \
    } while (0)
