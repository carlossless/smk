#pragma once

#include "sfr.h"

#define UART_HW_PRESENT 1

// TXD P0.0, RXD P0.1.
#define UART_VECTOR  _INT_EUART
#define UART_IEN     IEN0
#define UART_IEN_BIT _ES

// 0x98-0x9d carry the USB block on SFR page 1, so a write that lands on the wrong page hits
// EP0CON and USBCON instead of the EUART.
#define UART_PAGE_ENTER()             \
    uint8_t uart_saved_page = INSCON; \
    sfr_page_0()
#define UART_PAGE_LEAVE() INSCON = uart_saved_page
