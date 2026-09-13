#pragma once

#include "sfr.h"

#define UART_HW_PRESENT 1

#define UART_VECTOR  _INT_EUART0
#define UART_IEN     IEN0
#define UART_IEN_BIT _ES0

#define UART_PAGE_ENTER()             \
    uint8_t uart_saved_page = INSCON; \
    sfr_page_0()
#define UART_PAGE_LEAVE() INSCON = uart_saved_page
