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

// Vectors with no driver, and the enable bit gating each. SDCC fills slots only up to
// the highest declared handler; past it a vector runs whatever follows the table.
#ifdef DEBUG_SINK_UART
#    define UNUSED_INTERRUPTS_EUART0(X)
#else
#    define UNUSED_INTERRUPTS_EUART0(X) X(euart0, _INT_EUART0, IEN1, _ES0)
#endif

#define UNUSED_INTERRUPTS(X)         \
    X(int3, _INT_INT3, IEN0, _EX3)   \
    X(int2, _INT_INT2, IEN0, _EX2)   \
    X(scm, _INT_SCM, IEN0, _ESCM)    \
    X(lpd, _INT_LPD, IEN0, _ELPD)    \
    X(spi, _INT_SPI, IEN0, _ESPI)    \
    X(pwm1, _INT_PWM1, IEN1, _EPWM1) \
    X(pwm2, _INT_PWM2, IEN1, _EPWM2) \
    X(pwm3, _INT_PWM3, IEN1, _EPWM3) \
    X(pwm4, _INT_PWM4, IEN1, _EPWM4) \
    UNUSED_INTERRUPTS_EUART0(X)

#define UNUSED_INTERRUPT_DECL(name, vector, ien, bit) void name##_unused_interrupt_handler(void) __interrupt(vector);
UNUSED_INTERRUPTS(UNUSED_INTERRUPT_DECL)
#undef UNUSED_INTERRUPT_DECL

// Reports a vector trapped since the last call. DEBUG builds only.
void interrupts_task(void);
