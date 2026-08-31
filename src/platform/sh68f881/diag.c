#include "diag.h"

#if DEBUG == 1

#    include "sh68f881.h"
#    include "console.h"
#    include "debug.h"
#    include "kbdef.h"
#    include "delay.h"
#    include <stdint.h>

// Information block layout in the address space MOVC sees while FAC is set; the same map a programmer reaches over ICP.
#    define INFO_CUSTOMER_ID     0x1000u
#    define INFO_OPERATION_NUM   0x1004u
#    define INFO_CODE_OPTION_LOW 0x1006u
#    define INFO_SECURITY        0x100Au
#    define INFO_SERIAL_NUMBER   0x103Cu
#    define INFO_PART_NUMBER     0x1209u
#    define INFO_ID_CODE         0x127Bu

#    define INFO_SECURITY_LEN 17u
#    define CODE_OPTION_LEN 4u // this part has no separate high option line

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
            info_read(INFO_CODE_OPTION_LOW, scratch, CODE_OPTION_LEN);
            dprintf("OPT ");
            emit_hex(scratch, CODE_OPTION_LEN);
            dprintf("\r\n");
            break;
        case 6: {
            // Read the matrix port configuration back: a column that never drives low
            // looks exactly like a key that is never pressed.
            uint8_t p0cr = P0CR, p2cr = P2CR, p4cr = P4CR, p2pcr = P2PCR, p4pcr = P4PCR;
            dprintf("CFG p0cr=%02x p2cr=%02x p4cr=%02x pu2=%02x pu4=%02x\r\n", p0cr, p2cr, p4cr, p2pcr, p4pcr);
            break;
        }
        case 8: {
            // Count Timer0 ticks across a delay the delay loop believes is 1 ms. On a
            // 24 MHz core with the classic /12 prescale that is 2000; anything else is
            // the factor every delay and timer reload here is out by.
            uint16_t ticks;
            __critical
            {
                TR0  = 0;
                TMOD = (TMOD & 0xF0u) | 0x01u; // timer 0, 16-bit
                TH0  = 0;
                TL0  = 0;
                TF0  = 0;
                TR0  = 1;
                delay_us(1000);
                TR0   = 0;
                ticks = (uint16_t)(((uint16_t)TH0 << 8) | TL0);
            }
            dprintf("CLK ticks=%u ovf=%u\r\n", ticks, (uint8_t)TF0);
            break;
        }
        case 7: {
            uint8_t saved_page = INSCON;
            sfr_page_1();
            uint8_t c6 = P6CR, c7 = P7CR, c8 = P8CR;
            INSCON = saved_page;
            dprintf("CFG p6cr=%02x p7cr=%02x p8cr=%02x\r\n", c6, c7, c8);
            break;
        }
        default:
            break;
    }
}

#    define DIAG_STEPS 9u

// Walk one column at a time and report any that sees a row pulled low. Only one column
// is ever driven: the LED matrix shares these pins, and asserting all of them together
// sinks enough current to brown the part out.
static void diag_probe_rows(void)
{
    static uint8_t col = 0;
    uint8_t        rows;

    __critical
    {
        uint8_t saved_page = INSCON;
        sfr_page_1();
        P6 = 0xFFu;
        P7 = 0xFFu;
        P8 = 0xFFu;
        if (col < 8) {
            P6 = (uint8_t) ~(1u << col);
        } else if (col < 16) {
            P7 = (uint8_t) ~(1u << (col - 8));
        } else {
            P8 = (uint8_t) ~(1u << (col - 16));
        }
        INSCON = saved_page;

        for (uint8_t i = 0; i < 100; i++) {
            // clang-format off
            __asm
                nop
            __endasm;
            // clang-format on
        }

        rows = (uint8_t)(((P2 >> 1) & 0x07u) | ((P4 >> 1) & 0x38u));

        saved_page = INSCON;
        sfr_page_1();
        P6 = 0xFFu;
        P7 = 0xFFu;
        P8 = 0xFFu;
        INSCON = saved_page;
    }

    if (rows != 0x3Fu) {
        dprintf("KEY c%02u r=%02x\r\n", col, rows);
    }


    if (++col >= MATRIX_COLS) {
        col = 0;
        // Heartbeat per completed sweep: silence otherwise cannot be told apart from a
        // probe that is not running.
        static uint8_t sweeps = 0;
        if ((++sweeps & 0x0Fu) == 0) {
            // Reprint the accumulated map rather than only on change: the console is not
            // necessarily attached at the moment a key goes down, and anything printed
            // before it attaches is lost to the ring buffer.
            // Only the columns that saw something: the full 24-entry dump overruns the
            // console ring buffer and comes out mangled.
            // Report what the rows physically read alongside the accumulated map: an
            // idle read of 3f means the lines are pulled up and simply never pulled
            // down, anything else points at the port setup rather than the scan.
            extern volatile uint8_t kb_seen[MATRIX_COLS];
            extern uint8_t          matrix[MATRIX_COLS];
            dprintf("MAP r=%02x m=%02x%02x%02x", rows, matrix[0], matrix[2], matrix[6]);
            for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                if (kb_seen[c]) {
                    dprintf(" %u:%02x", c, kb_seen[c]);
                }
            }
            dprintf("\r\n");
        }
    }
}

void diag_task(void)
{
    static uint8_t step = 0;

    if (step >= DIAG_STEPS) {
        if (console_is_drained()) {
            diag_probe_rows();
        }
        return;
    }
    // Emitting only into a drained buffer paces the dump against the host link, so no line is ever dropped however long the dump grows.
    if (!console_is_drained()) {
        return;
    }
    diag_emit(step++);
}

#endif // DEBUG
