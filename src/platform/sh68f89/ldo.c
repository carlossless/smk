#include "ldo.h"
#include "sh68f89.h"

void ldo_init()
{
    REGCON = REGCON_ENABLE;
}
