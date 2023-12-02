#include "isp.h"
#include "sh68f90a.h"

// *INDENT-OFF*
void isp_jump() __naked
{
    __asm
    clr IE.7
    mov B, #0xa5
    mov A, #0x5a
    ljmp 0xff00
    __endasm;
}
// *INDENT-ON*
