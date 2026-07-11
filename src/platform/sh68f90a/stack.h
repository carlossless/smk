#pragma once

// Stack high-water measurement (DEBUG only). stack_paint() must run once at
// the very start of main() after init() returns, while the stack is still
// shallow; it fills unused iRAM with a sentinel. stack_task(), called from
// the main loop, scans down from STACK_TOP and reports new peaks over the
// HID debug console (`SPpk <hex>` = bytes used at peak).

void stack_paint(void);
void stack_task(void);
