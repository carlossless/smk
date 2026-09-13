#pragma once

#include "sfr.h"

#define PWM_HW_PRESENT 1

// Five banks of six channels, named PWM<bank><channel>CON with DUTY1/DUTY2 register pairs
// beside them, which is the naming pwm.h's macros paste together. Which channels a board
// drives is the board's own business; the clock divider is not, so it is settled in pwm.h.
#define PWM_VECTOR _INT_PWM0
