#pragma once

#include "sh68f90a.h"
#include <stdbool.h>

void power_enter_powerdown(bool usb_keep_alive);

void int4_isr(void) __interrupt(_INT_INT4);
