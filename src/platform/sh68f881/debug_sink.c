#include "console.h"

// the EUART has no driver here, so the HID console is the only sink.
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
