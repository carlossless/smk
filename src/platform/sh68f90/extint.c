#include "extint.h"
#include "sh68f90.h"

// INT4's edge and trigger-mode encodings. Transcribed from the stock firmware's
// sleep entry - the datasheet does not document these two registers, so the
// values are known to work rather than known to be minimal.
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
