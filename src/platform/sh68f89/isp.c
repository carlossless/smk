#include "isp.h"
#include "sh68f89.h"

void isp_jump() __naked
{
    // clang-format off
    __asm
    clr IE.7
    mov B, #0xa5
    mov A, #0xea
    ljmp 0xff00
    __endasm;
    // clang-format on
}
