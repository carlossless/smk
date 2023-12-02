#include "sh68f90a.h"

void ldo_init()
{
    REGCON = _REGEN; // enable 3v3 ldo
}
