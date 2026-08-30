#include "diag.h"

#if DEBUG == 1

#    include "sh68f90a.h"
#    include "console.h"
#    include "debug.h"
#    include <stdint.h>

// Information block layout in the address space MOVC sees while FAC is set; the same map a programmer reaches over ICP.
#    define INFO_CUSTOMER_ID     0x1000u
#    define INFO_OPERATION_NUM   0x1004u
#    define INFO_CODE_OPTION_LOW 0x1006u
#    define INFO_SECURITY        0x100Au
#    define INFO_SERIAL_NUMBER   0x103Cu
#    define INFO_CODE_OPTION_HI  0x1100u
#    define INFO_PART_NUMBER     0x1209u
#    define INFO_ID_CODE         0x127Bu

#    define INFO_SECURITY_LEN 17u
#    define CODE_OPTION_LEN   8u
#    define CODE_OPTION_SPLIT 4u // bytes 0-3 sit with the customer fields, 4-7 stand alone

// FLASHCON.FAC: point MOVC at the information block.
#    define FLASHCON_FAC 0x01u

static __xdata uint8_t scratch[INFO_SECURITY_LEN];

static void info_read(uint16_t addr, __xdata uint8_t *dst, uint8_t len)
{
    // While FAC is set every MOVC hits the information block, so an ISR firing here would fetch its __code reads from the wrong place.
    __critical
    {
        FLASHCON = FLASHCON_FAC;
        for (uint8_t i = 0; i < len; i++) {
            __code uint8_t *p = (__code uint8_t *)(addr + i);
            dst[i]            = *p;
        }
        FLASHCON = 0;
    }
}

static void emit_hex(const __xdata uint8_t *src, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        dprintf("%02x", src[i]);
    }
}

static void diag_emit(uint8_t step)
{
    switch (step) {
        case 0:
            info_read(INFO_PART_NUMBER, scratch, 5);
            dprintf("PART ");
            emit_hex(scratch, 5);
            dprintf("\r\n");
            break;
        case 1:
            info_read(INFO_ID_CODE, scratch, 5);
            dprintf("UID ");
            emit_hex(scratch, 5);
            dprintf("\r\n");
            break;
        case 2:
            info_read(INFO_CUSTOMER_ID, scratch, 4);
            dprintf("CID ");
            emit_hex(scratch, 4);
            info_read(INFO_OPERATION_NUM, scratch, 2);
            dprintf(" OPN ");
            emit_hex(scratch, 2);
            dprintf("\r\n");
            break;
        case 3:
            info_read(INFO_SERIAL_NUMBER, scratch, 4);
            dprintf("SN ");
            emit_hex(scratch, 4);
            dprintf("\r\n");
            break;
        case 4:
            info_read(INFO_SECURITY, scratch, INFO_SECURITY_LEN);
            dprintf("SEC ");
            emit_hex(scratch, INFO_SECURITY_LEN);
            dprintf("\r\n");
            break;
        case 5:
            info_read(INFO_CODE_OPTION_LOW, scratch, CODE_OPTION_SPLIT);
            info_read(INFO_CODE_OPTION_HI, scratch + CODE_OPTION_SPLIT, CODE_OPTION_LEN - CODE_OPTION_SPLIT);
            dprintf("OPT ");
            emit_hex(scratch, CODE_OPTION_LEN);
            dprintf("\r\n");
            break;
        default:
            break;
    }
}

#    define DIAG_STEPS 6u

void diag_task(void)
{
    static uint8_t step = 0;

    if (step >= DIAG_STEPS) {
        return;
    }
    // Emitting only into a drained buffer paces the dump against the host link, so no line is ever dropped however long the dump grows.
    if (!console_is_drained()) {
        return;
    }
    diag_emit(step++);
}

#endif // DEBUG
