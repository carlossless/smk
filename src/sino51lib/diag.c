#include "diag.h"

#if DEBUG == 1

#    include "flash.h"
#    include "console.h"
#    include "debug.h"
#    include <stdint.h>

__xdata uint8_t diag_scratch[DIAG_SCRATCH_SIZE];

void diag_info_read(uint16_t addr, __xdata uint8_t *dst, uint8_t len)
{
    flash_read_into(FLASH_DATA, addr, dst, len);
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

#endif
