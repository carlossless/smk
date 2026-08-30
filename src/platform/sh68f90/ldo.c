#include "sh68f90.h"

void ldo_init()
{
    REGCON = _REGEN; // enable 3v3 ldo
}
