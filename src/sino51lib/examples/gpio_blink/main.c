// The smallest thing that uses the library: reset status, regulator, clock, pin mux, GPIO,
// the delay loop and the watchdog. Links no part of smk.
//
// GPIO_OUTPUT and friends paste the port number into a register name, so the port has to be
// written as a literal digit; P3.0 here is a placeholder, pick a pin your board leaves free.

#include "clock.h"
#include "delay.h"
#include "gpio.h"
#include "ldo.h"
#include "peripherals.h"
#include "reset.h"
#include "watchdog.h"

#define BLINK_BIT 0x01u
#define BLINK_MS  250u

void main(void)
{
    reset_init();
    ldo_init();
    clock_init();
    peripherals_init();

    GPIO_OUTPUT(3, BLINK_BIT);

    for (;;) {
        watchdog_kick();
        GPIO_HIGH(3, BLINK_BIT);
        delay_ms(BLINK_MS);

        watchdog_kick();
        GPIO_LOW(3, BLINK_BIT);
        delay_ms(BLINK_MS);
    }
}
