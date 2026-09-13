// The EUART, on whichever pins the part puts it (see its uart_hw.h). The driver hands the
// byte to the shift register and waits for the transmit-complete interrupt, so the vector has
// to be declared here: SDCC builds the table from what the module holding main() can see.
//
// uart.c is compiled only when DEBUG_SINK_UART is defined, which is the build's name for
// "send debug output to the serial port"; the example defines it for itself.

#include "clock.h"
#include "delay.h"
#include "ldo.h"
#include "peripherals.h"
#include "uart.h"
#include "uart_hw.h"
#include "watchdog.h"

void uart_interrupt_handler(void) __interrupt(UART_VECTOR);

static void put(const char *s)
{
    while (*s) {
        uart_putc((unsigned char)*s++);
    }
}

void main(void)
{
    ldo_init();
    clock_init();
    peripherals_init();
    uart_init();

    EA = 1;

    for (;;) {
        watchdog_kick();
        put("sino51lib\r\n");
        delay_ms(1000);
    }
}
