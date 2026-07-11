#pragma once

#include "sh68f90a.h"

// Single-ISR design. Timer 2 fires periodically (default ~5 kHz). Each fire is
// either a full matrix scan (1 in every 22 fires) or one LED PWM substep (the
// other 21):
//
//   phase == 0  →  matrix scan, advance to phase 1.
//   phase == 1  →  LED PWM substep N. When N wraps from 20 → 0, phase
//                  flips back to 0 and the next fire is a matrix scan.
//
// The 1:21 ratio gives a matrix-scan rate ≈ T2_rate/22 (≈ 227 Hz at
// 5 kHz T2) and a full LED frame refresh ≈ same rate (because there
// are 21 LED substeps per frame, and one of every 22 ticks is matrix).
void timer2_init(void);

// Pause / resume the scan ISR (masks the Timer 2 interrupt; the timer keeps
// counting). Used to keep the LED columns quiescent across a multi-millisecond
// blocking flash write: with the scan running it would re-arm the column PWM
// between flash ops, undoing a pre-save park before the erase's interrupts-off
// stall (-> a bright "blip" on whatever row was active). The matrix scan also
// pauses, but only for the ~5 ms save — keys are re-sampled on resume.
void timer2_scan_pause(void);
void timer2_scan_resume(void);

// Prototype must live in a header that main.c includes, otherwise SDCC
// won't emit the vector slot.
void timer2_interrupt_handler(void) __interrupt(_INT_TIMER2);
