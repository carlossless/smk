#pragma once

#include "sh68f90a.h"

typedef enum {
    POWERDOWN_KEEP_USB_ALIVE,
    POWERDOWN_RELEASE_USB,
} powerdown_mode_t;

void power_enter_powerdown(powerdown_mode_t mode);
