#include "sh68f90a.h"

#define _3V3_LDO_EN (1 << 0) // enable 3.3V LDO, necessary for USB (RAIN)
#define REGCON_INIT (_3V3_LDO_EN)

void ldo_init()
{
    REGCON = REGCON_INIT; // enable 3v3
}
