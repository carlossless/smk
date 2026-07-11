#pragma once

#include "sh68f90a.h"

// Single-ISR design. Timer 2 fires periodically; each fire is either a matrix
// scan (1 in every 22) or one LED PWM substep (the other 21). phase 0 runs the
// scan, phase 1 runs substep N; when N wraps 20 → 0, phase flips back to 0.
void timer2_init(void);

// Pause / resume the scan ISR (masks the Timer 2 interrupt; the timer keeps
// counting). Keeps the LED columns quiescent across a blocking flash write —
// otherwise the scan re-arms the column PWM between flash ops, undoing the
// pre-save park and blipping whatever row was active. Matrix scan also pauses
// for the ~5 ms save; keys re-sample on resume.
void timer2_scan_pause(void);
void timer2_scan_resume(void);

// Prototype must be visible where main.c is compiled, else SDCC won't emit the
// vector slot.
void timer2_interrupt_handler(void) __interrupt(_INT_TIMER2);
