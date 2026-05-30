#pragma once

// Stack high-water measurement (DEBUG only). stack_paint() must run once at the
// very start of main(), before interrupts and any deep calls; it fills the
// unused stack with a sentinel. stack_task(), called from the main loop, scans
// for the highest byte the stack ever reached and reports new peaks over the
// HID debug console. See [[project-iram-limit]] for why this matters.

void stack_paint(void);
void stack_task(void);
