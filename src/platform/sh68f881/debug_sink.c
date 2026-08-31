#include "console.h"

// The SH68F90 selects between the UART and HID console in its uart.c; this platform
// has no UART driver, so the console is the only sink.
void debug_putc(char c)
{
#ifdef DEBUG_SINK_CONSOLE
    console_putc((unsigned char)c);
#endif
    (void)c;
}

void putchar(int c)
{
    debug_putc((char)c);
}
