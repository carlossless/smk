#include "ldo.h"
#include "sh68f881.h"

void ldo_init()
{
    REGCON = REGCON_ENABLE;
}
