// The PWM banks. pwm.h only pastes register names together, so there is nothing to link: the
// bank and channel names come from the part's header, and which channels a board actually
// wires is the board's business. pwm.c holds only the safety-net ISR for a bank left with its
// interrupt enabled, which this example does not do, so it is not linked either.

#include "clock.h"
#include "delay.h"
#include "ldo.h"
#include "peripherals.h"
#include "pwm.h"
#include "watchdog.h"

#define DUTY_PERIOD 0x00FFu

void main(void)
{
    ldo_init();
    clock_init();
    peripherals_init();

    PWM00CON = PWM_CON_PARKED;
    SET_PWM_DUTY(PWM00, DUTY_PERIOD, 0x0000);

    for (uint16_t duty = 0;; duty += 0x10) {
        watchdog_kick();
        SET_PWM_DUTY_2(PWM00, duty & DUTY_PERIOD);
        delay_ms(20);
    }
}
