#include "ldo.h"
#include "sh68f90.h"

// RSTSTAT is also the watchdog kick, so a kick destroys the reset-source flags. This is the
// first platform call, which is the only place they can still be read.
uint8_t reset_status;

void ldo_init()
{
    reset_status = RSTSTAT;
    REGCON       = _REGEN;
}
