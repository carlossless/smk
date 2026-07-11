#pragma once

#include "sh68f90a.h"
#include <stdbool.h>

// Put the SH68F90A into Power-Down mode (PCON.PD) and return once a wake source
// fires. The caller MUST arm a wake source first (the board hook arms INT4) — in
// Power-Down only external interrupts (INT2/3/4), LPD, USB bus events, or reset
// can wake the core (datasheet 8.9.3). This routine owns the generic clock/PLL/
// USB/regulator teardown + the SUSLO/PCON.PD dance + the wake rebuild.
//
// usb_keep_alive: true = USB-suspend variant (regulator stays on, GOSUSP, resume
// via remote-wakeup, no re-enumerate) for when the host has suspended the bus;
// false = RF/battery variant (regulator off, full USB re-init on wake).
void power_enter_powerdown(bool usb_keep_alive);

// INT4 (external interrupt 4) service routine — the wake-from-Power-Down vector.
// Declared here so main.c can pull the prototype into the translation unit that
// owns the ISR vector table (SDCC only emits the vector when the __interrupt
// prototype is visible where main() is compiled — same rule as timer2/pwm).
void int4_isr(void) __interrupt(_INT_INT4);
