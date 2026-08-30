#pragma once

#include "sh68f90.h"

// SDCC only emits a vector slot for a handler whose prototype is visible in the
// translation unit that defines main(), so main.c includes this instead of every
// driver header it would otherwise have no reason to know about.
//
// No handler claims a register bank with __using(N): they call ordinary bank-0
// code, and a reserved bank costs DSEG this part cannot spare.

void systick_interrupt_handler(void) __interrupt(_INT_TIMER2);
void usb_interrupt_handler(void) __interrupt(_INT_USB);
void int4_interrupt_handler(void) __interrupt(_INT_INT4);
#ifdef DEBUG_SINK_UART
void uart_interrupt_handler(void) __interrupt(_INT_EUART0);
#endif
void pwm_interrupt_handler(void) __interrupt(_INT_PWM0);
