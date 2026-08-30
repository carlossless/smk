#pragma once

#include "sh68f881.h"

// SDCC only emits a vector slot for a handler whose prototype is visible in the
// translation unit that defines main().

void usb_interrupt_handler(void) __interrupt(_INT_USB);
