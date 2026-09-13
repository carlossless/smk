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
