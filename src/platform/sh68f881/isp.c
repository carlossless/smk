#include "isp.h"
#include "sh68f881.h"

// The bootloader entry requires A == 0x5a and B == 0xa5, and falls through harmlessly otherwise.
void isp_jump() __naked
{
    // clang-format off
    __asm
    clr IE.7
    mov B, #0xa5
    mov A, #0x5a
    ljmp 0x7f00
    __endasm;
    // clang-format on
}
