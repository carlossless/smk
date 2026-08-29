#pragma once

#include "sh68f90a.h"

// Port drive strength lives in the per-port PxDRV registers, which are
// write-gated by DRVCON: the high nibble selects the port and 0x5 is the unlock
// key. Clear DRVCON when done so a later PxDRV write can't take effect by
// accident.
#define DRVCON_UNLOCK_P1 0x05
#define DRVCON_UNLOCK_P2 0x45
#define DRVCON_UNLOCK_P3 0x85
#define DRVCON_UNLOCK_P5 0xc5
#define DRVCON_LOCK      0x00

#define GPIO_DRIVE_25MA 0x00
