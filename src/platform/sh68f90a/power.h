#pragma once

#include "sh68f90a.h"
#include <stdbool.h>

// Enter Power-Down (PCON.PD) and return once a wake source fires. The caller
// MUST arm a wake source first (the board hook arms INT4) — in Power-Down only
// INT2/3/4, LPD, USB bus events, or reset can wake the core (datasheet 8.9.3).
// Owns the generic clock/PLL/USB/regulator teardown + wake rebuild.
//
// usb_keep_alive: true = USB-suspend variant (regulator on, GOSUSP, resume via
// remote-wakeup, no re-enumerate); false = RF/battery (regulator off, full USB
// re-init on wake).
void power_enter_powerdown(bool usb_keep_alive);

// INT4 service routine — the wake-from-Power-Down vector. Declared here so the
// prototype is visible where main() is compiled (SDCC only emits the vector
// then).
void int4_isr(void) __interrupt(_INT_INT4);
