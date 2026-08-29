#include "extint.h"
#include "sh68f90a.h"

#define IENC_INT4_CONFIG  0xF3
#define EXF0_INT4_TRIGGER 0x40

void extint_wake_clear(void)
{
    EXF1 = 0;
}

void extint_wake_arm(void)
{
    extint_wake_clear();
    IENC = IENC_INT4_CONFIG;
    EXF0 = EXF0_INT4_TRIGGER;
    EX4  = 1;
}

void extint_wake_disable(void)
{
    EX4 = 0;
    extint_wake_clear();
}
