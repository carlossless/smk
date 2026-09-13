#include "clock.h"
#include "delay.h"
#include "gpio.h"
#include "ldo.h"
#include "peripherals.h"
#include "systick.h"
#include "tick.h"
#include "watchdog.h"
#include <stdint.h>

void systick_interrupt_handler(void) __interrupt(_INT_TIMER2);

static volatile uint16_t ticks;

void tick_dispatch(void)
{
    ticks++;
}

void main(void)
{
    ldo_init();
    clock_init();
    peripherals_init();

    GPIO_OUTPUT(3, 0x01u);

    systick_init();
    EA = 1;

    for (;;) {
        watchdog_kick();

        systick_arm(SYSTICK_SLOT_MATRIX_SCAN);
        delay_ms(100);

        systick_arm(SYSTICK_SLOT_LED_SUBFRAME);
        delay_ms(100);

        systick_pause();
        GPIO_WRITE(3, (uint8_t)(ticks >> 8));
        systick_resume();
    }
}
