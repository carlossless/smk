#include "debug.h"
#include "console.h"
#ifdef DEBUG_SINK_UART
#    include "uart.h"
#endif

void debug_putc(char c)
{
#ifdef DEBUG_SINK_UART
    uart_putc((unsigned char)c);
#endif
#ifdef DEBUG_SINK_CONSOLE
    console_putc((unsigned char)c);
#endif
    (void)c;
}

void putchar(int c)
{
    debug_putc((char)c);
}
