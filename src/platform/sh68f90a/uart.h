#ifndef UART_H
#define UART_H

#include "sh68f90a.h"

void          uart_init();
void          uart_putc(unsigned char c);
unsigned char uart_getc();
void          uart_interrupt_handler() __interrupt(_INT_EUART0);

#endif
