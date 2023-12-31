#ifndef PWM_H
#define PWM_H

#include "sh68f90a.h"
#include <stdint.h>

void pwm_init();
void pwm_enable();
void pwm_disable();
void pwm_set_all_columns(uint16_t intensity);
void pwm_interrupt_handler() __interrupt(_INT_PWM0);

#endif
