#include "ldo.h"
#include "sfr.h"

void ldo_init()
{
    REGCON = REGCON_ENABLE;
}
