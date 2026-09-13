#include "ldo.h"
#include "sh68f90.h"

void ldo_init()
{
    REGCON = REGCON_ENABLE;
}
