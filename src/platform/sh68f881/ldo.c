#include "ldo.h"
#include "sh68f881.h"

// the watchdog kick overwrites RSTSTAT's reset-source flags, so latch them at the first platform call.
uint8_t reset_status;

void ldo_init()
{
    reset_status = RSTSTAT;
}
