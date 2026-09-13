#include "reset.h"
#include "sh68f89.h"

uint8_t reset_status;

void reset_init(void)
{
    reset_status = RSTSTAT;
}
