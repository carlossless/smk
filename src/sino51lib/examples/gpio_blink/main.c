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
#include <stdbool.h>
#include <stdint.h>

#define BLINK_BIT 0x01u
#define SENSE_BIT 0x02u
#define BLINK_MS  250u

void main(void)
{
    reset_init();
    ldo_init();
    clock_init();
    peripherals_init();

    // a pin read back: input, pull-up on, sample, then hand it to the output side
    GPIO_INPUT(3, SENSE_BIT);
    GPIO_PULLUP_ON(3, SENSE_BIT);
    delay_us(50);
    bool sense_high = (P3 & SENSE_BIT) != 0;
    GPIO_PULLUP_OFF(3, SENSE_BIT);

    GPIO_DIR_WRITE(3, BLINK_BIT);
    GPIO_PULLUP_WRITE(3, 0x00u);
    GPIO_WRITE(3, 0x00u);

    // reset_status is whatever RSTSTAT held before anything kicked the watchdog
    uint16_t period = (reset_status != 0 && sense_high) ? BLINK_MS : BLINK_MS / 2u;

    for (;;) {
        watchdog_kick();
        GPIO_HIGH(3, BLINK_BIT);
        delay_ms(period);

        watchdog_kick();
        GPIO_LOW(3, BLINK_BIT);
        delay_ms(period);
    }
}
