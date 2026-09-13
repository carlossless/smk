#include "pwm_hw.h"

#if PWM_HW_PRESENT

#    include "interrupts.h"

// Safety net for a bank left with IE set; nothing is scheduled off this vector.
void pwm_interrupt_handler(void) __interrupt(PWM_VECTOR) {}

#endif
