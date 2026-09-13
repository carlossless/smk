#include "clock.h"
#include "delay.h"
#include "ldo.h"
#include "peripherals.h"
#include "pwm.h"
#include "pwm_hw.h"
#include "watchdog.h"

void pwm_interrupt_handler(void) __interrupt(PWM_VECTOR);

#define DUTY_PERIOD 0x00FFu

#define PWM_CON_RUN (uint8_t)(PWM_MODE_ENABLE | PWM_INT_ENABLE | PWM_MOD | PWM_CLK_DIV_4)

void main(void)
{
    ldo_init();
    clock_init();
    peripherals_init();

    PWM00CON = PWM_CON_PARKED;
    SET_PWM_DUTY(PWM00, DUTY_PERIOD, 0x0000);

    PWM00CON = PWM_CON_RUN;
    EA       = 1;

    for (uint16_t duty = 0;; duty += 0x10) {
        watchdog_kick();
        SET_PWM_DUTY_1(PWM00, DUTY_PERIOD);
        SET_PWM_DUTY_2(PWM00, duty & DUTY_PERIOD);
        delay_ms(20);
    }
}
