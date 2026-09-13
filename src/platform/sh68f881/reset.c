#include "reset.h"
#include "sh68f881.h"

uint8_t reset_status;

void reset_init(void)
{
    reset_status = RSTSTAT;
}
