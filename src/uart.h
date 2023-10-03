#ifndef _UART_H_
#define _UART_H_

void uart_init();
void uart_putc(unsigned char c);
unsigned char uart_getc();
void uart_interrupt_handler() __interrupt (13);

#endif
