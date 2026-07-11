#pragma once

#include "sh68f90a.h"

void timer2_init(void);

void timer2_scan_pause(void);
void timer2_scan_resume(void);

void timer2_interrupt_handler(void) __interrupt(_INT_TIMER2);
