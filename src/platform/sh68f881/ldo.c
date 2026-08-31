#include "ldo.h"
#include "sh68f881.h"

// RSTSTAT still carries the reset-source flags here, and the watchdog kick overwrites
// them, so latch it at the very first platform call for the boot diagnostic.
uint8_t reset_status;

// The regulator on this part is REGCON, which clock_init already sets alongside the
// PLL because the USB block will not run without both.
void ldo_init()
{
    reset_status = RSTSTAT;
}
