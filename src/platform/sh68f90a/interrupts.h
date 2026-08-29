#pragma once

#include "sh68f90a.h"

void systick_interrupt_handler(void) __interrupt(_INT_TIMER2);
void usb_interrupt_handler(void) __interrupt(_INT_USB);
void int4_interrupt_handler(void) __interrupt(_INT_INT4);
#ifdef DEBUG_SINK_UART
void uart_interrupt_handler(void) __interrupt(_INT_EUART0);
#endif
void pwm_interrupt_handler(void) __interrupt(_INT_PWM0);
