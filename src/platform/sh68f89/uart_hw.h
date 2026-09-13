#pragma once

#include "sfr.h"

#define UART_HW_PRESENT 1

// EUART0: TXD P6.4, RXD P6.5. EUART1, on P4.6/P4.7, is left unmapped.
#define UART_VECTOR  _INT_EUART0
#define UART_IEN     IEN0
#define UART_IEN_BIT _ES0

// 0x98-0x9e carry PCA0 on SFR page 1, so a write that lands on the wrong page hits the
// backlight timers instead of the EUART.
#define UART_PAGE_ENTER()             \
    uint8_t uart_saved_page = INSCON; \
    sfr_page_0()
#define UART_PAGE_LEAVE() INSCON = uart_saved_page
