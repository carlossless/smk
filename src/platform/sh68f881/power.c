#include "power.h"
#include "sh68f881.h"

// Power-down is not brought up on this part yet. Returning without entering it costs
// idle current but can never strand the board asleep with no working wake source.
void power_enter_powerdown(powerdown_mode_t mode)
{
    (void)mode;
}
