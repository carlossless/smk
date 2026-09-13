#include "diag.h"

#if DEBUG == 1

#    include "sfr.h"
#    include "console.h"
#    include "debug.h"
#    include <stdint.h>

__xdata uint8_t diag_scratch[DIAG_SCRATCH_SIZE];

void diag_info_read(uint16_t addr, __xdata uint8_t *dst, uint8_t len)
{
    // while FAC is set every MOVC hits the information block, so an ISR firing here would fetch its __code reads from the wrong place.
    __critical
    {
        FLASHCON = _FAC;
        for (uint8_t i = 0; i < len; i++) {
            __code uint8_t *p = (__code uint8_t *)(addr + i);
            dst[i]            = *p;
        }
        FLASHCON = 0;
    }
}

void diag_emit_hex(const __xdata uint8_t *src, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        dprintf("%02x", src[i]);
    }
}

#    define DIAG_DONE 0xFFu

void diag_task(void)
{
    static uint8_t step = 0;

    if (step == DIAG_DONE) {
        return;
    }
    // emitting only into a drained buffer paces the dump against the host link, so no line is ever dropped however long the dump grows.
    if (!console_is_drained()) {
        return;
    }
    if (!diag_emit(step)) {
        step = DIAG_DONE;
        return;
    }
    step++;
}

#endif // DEBUG
