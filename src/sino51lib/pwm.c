#include "pwm_hw.h"

#if PWM_HW_PRESENT

#    include "interrupts.h"

void pwm_interrupt_handler(void) __interrupt(PWM_VECTOR) {}

#endif
