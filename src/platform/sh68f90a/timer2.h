#pragma once

#include "sh68f90a.h"

// Stock-faithful single-ISR design. Timer 2 fires periodically (default
// ~5 kHz). Each fire is either a full matrix scan (1 in every 22 fires)
// or one LED PWM substep (the other 21). This mirrors the stock fw's
// `isr_timer2_pwm_anim` at CODE:0x0003 which gates on bit `_8_4`:
//
//   phase == 0  →  matrix scan, advance to phase 1.
//   phase == 1  →  LED PWM substep N. When N wraps from 20 → 0, phase
//                  flips back to 0 and the next fire is a matrix scan.
//
// The 1:21 ratio gives a matrix-scan rate ≈ T2_rate/22 (≈ 227 Hz at
// 5 kHz T2) and a full LED frame refresh ≈ same rate (because there
// are 21 LED substeps per frame, and one of every 22 ticks is matrix).
void timer2_init(void);

// Prototype must live in a header that main.c includes, otherwise SDCC
// won't emit the vector slot.
void timer2_interrupt_handler(void) __interrupt(_INT_TIMER2);
