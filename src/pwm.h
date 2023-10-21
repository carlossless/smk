#ifndef _PWM_H_
#define _PWM_H_

#include <stdint.h>

void pwm_init();
void pwm_enable();
void pwm_disable();
void pwm_set_all_columns(uint16_t intensity);
void pwm_interrupt_handler() __interrupt (8);

#endif
