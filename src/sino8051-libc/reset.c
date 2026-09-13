#include "reset.h"
#include "sfr.h"

uint8_t reset_status;

void reset_init(void)
{
    reset_status = RSTSTAT;
}
