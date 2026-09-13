#include "isp.h"
#include "sfr.h"

void isp_jump() __naked
{
    // clang-format off
    __asm
    clr IE.7
    mov B, #ISP_KEY_B
    mov A, #ISP_KEY_A
    ljmp ISP_ENTRY
    __endasm;
    // clang-format on
}
