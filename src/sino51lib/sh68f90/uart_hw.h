#pragma once

#include "sfr.h"

#define UART_HW_PRESENT 1

#define UART_VECTOR  _INT_EUART0
#define UART_IEN     IEN1
#define UART_IEN_BIT _ES0

#define UART_PAGE_ENTER() ((void)0)
#define UART_PAGE_LEAVE() ((void)0)
