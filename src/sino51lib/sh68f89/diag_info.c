#include "diag.h"

#if DEBUG == 1

#    include "sh68f89.h"
#    include "debug.h"
#    include <stdint.h>

// information block layout in the address space MOVC sees while FAC is set; the same map a programmer reaches over ICP.
#    define INFO_CUSTOMER_ID     0x1000u
#    define INFO_OPERATION_NUM   0x1004u
#    define INFO_CODE_OPTION_LOW 0x1006u
#    define INFO_SECURITY        0x100Au
#    define INFO_SERIAL_NUMBER   0x103Cu
#    define INFO_PART_NUMBER     0x1209u
#    define INFO_ID_CODE         0x127Bu

#    define INFO_SECURITY_LEN 17u
#    define CODE_OPTION_LEN   4u // this part has no separate high option line

bool diag_emit(uint8_t step)
{
    switch (step) {
        case 0:
            diag_info_read(INFO_PART_NUMBER, diag_scratch, 5);
            dprintf("PART ");
            diag_emit_hex(diag_scratch, 5);
            dprintf("\r\n");
            break;
        case 1:
            diag_info_read(INFO_ID_CODE, diag_scratch, 5);
            dprintf("UID ");
            diag_emit_hex(diag_scratch, 5);
            dprintf("\r\n");
            break;
        case 2:
            diag_info_read(INFO_CUSTOMER_ID, diag_scratch, 4);
            dprintf("CID ");
            diag_emit_hex(diag_scratch, 4);
            diag_info_read(INFO_OPERATION_NUM, diag_scratch, 2);
            dprintf(" OPN ");
            diag_emit_hex(diag_scratch, 2);
            dprintf("\r\n");
            break;
        case 3:
            diag_info_read(INFO_SERIAL_NUMBER, diag_scratch, 4);
            dprintf("SN ");
            diag_emit_hex(diag_scratch, 4);
            dprintf("\r\n");
            break;
        case 4:
            diag_info_read(INFO_SECURITY, diag_scratch, INFO_SECURITY_LEN);
            dprintf("SEC ");
            diag_emit_hex(diag_scratch, INFO_SECURITY_LEN);
            dprintf("\r\n");
            break;
        case 5:
            diag_info_read(INFO_CODE_OPTION_LOW, diag_scratch, CODE_OPTION_LEN);
            dprintf("OPT ");
            diag_emit_hex(diag_scratch, CODE_OPTION_LEN);
            dprintf("\r\n");
            break;
        case 6: {
            // read the matrix port configuration back: a column that never drives low looks exactly like a key that is never pressed.
            uint8_t p0cr = P0CR, p1cr = P1CR, p4cr = P4CR, pu0 = P0PCR, pu1 = P1PCR;
            dprintf("CFG p0cr=%02x p1cr=%02x p4cr=%02x pu0=%02x pu1=%02x\r\n", p0cr, p1cr, p4cr, pu0, pu1);
            break;
        }
        case 7: {
            uint8_t saved_page = INSCON;
            sfr_page_1();
            uint8_t c5 = P5CR, c7 = P7CR, u5 = P5PCR, u7 = P7PCR;
            INSCON = saved_page;
            dprintf("CFG p5cr=%02x p7cr=%02x pu5=%02x pu7=%02x\r\n", c5, c7, u5, u7);
            break;
        }
        default:
            return false;
    }
    return true;
}

#endif // DEBUG
