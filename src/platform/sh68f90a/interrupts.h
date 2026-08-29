#pragma once

#include "sh68f90a.h"

// The interrupt vector table, in one place.
//
// SDCC only emits a vector slot for a handler whose prototype is visible in the
// translation unit that defines main(), so main.c includes this instead of every
// driver header it would otherwise have no reason to know about.
//
// None of these claim a private register bank with __using(N): a handler tagged
// that way may only call functions tagged the same way, and these reach ordinary
// bank-0 code (the matrix sweep, the LED update). A reserved bank also costs 8
// bytes of DSEG this part cannot spare, so the default push/pop of R0-R7 stands.

void systick_interrupt_handler(void) __interrupt(_INT_TIMER2);
void usb_interrupt_handler(void) __interrupt(_INT_USB);
void int4_interrupt_handler(void) __interrupt(_INT_INT4);
#ifdef DEBUG_SINK_UART
void uart_interrupt_handler(void) __interrupt(_INT_EUART0);
#endif
// PWM0's period interrupt. The handler is empty — nothing schedules work off it,
// the systick ISR owns the LED scan — but eyooso-z11 still sets both enable bits
// (PWM00CON.IE and IEN1.EPWM0), so the slot must exist and return cleanly.
void pwm_interrupt_handler(void) __interrupt(_INT_PWM0);
