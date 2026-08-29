#pragma once

#include "sh68f90a.h"

// What the core is allowed to keep alive across a Power-Down.
typedef enum {
    // The host parked the bus itself. Keep the regulator and PHY powered so the
    // device stays enumerated and can resume via remote-wakeup.
    POWERDOWN_KEEP_USB_ALIVE,
    // Running on battery. Power the USB regulator and PHY all the way down; the
    // host re-enumerates when we come back.
    POWERDOWN_RELEASE_USB,
} powerdown_mode_t;

// Enter Power-Down and return once a wake source fires. The caller MUST arm a
// wake source first (the board hook arms INT4) — in Power-Down only INT2/3/4,
// LPD, USB bus events, or reset can wake the core (datasheet 8.9.3). Owns the
// generic clock/PLL/USB/regulator teardown and the wake rebuild.
void power_enter_powerdown(powerdown_mode_t mode);
