#pragma once

#include "sh68f881.h"

// SDCC only emits a vector slot for a handler whose prototype is visible in the
// translation unit that defines main().

void systick_interrupt_handler(void) __interrupt(_INT_TIMER2);
void usb_interrupt_handler(void) __interrupt(_INT_USB);
