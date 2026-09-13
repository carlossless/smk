#include <stdint.h>

// SDCC's genXINIT copies XINIT with paged MOVX, but P2 is a GPIO port here, so initialized __xdata needs this DPTR redo.
uint8_t __sdcc_external_startup(void) __naked
{
    // clang-format off
    __asm
        mov     r1, #l_XINIT                    ; 16-bit byte count, low
        mov     a,  r1
        orl     a,  #(l_XINIT >> 8)
        jz      00003$                          ; nothing to copy
        mov     r2, #((l_XINIT + 255) >> 8)     ; ...and its page count

        mov     r3, #s_XINIT                    ; src cursor (code); #sym = low byte
        mov     r4, #(s_XINIT >> 8)
        mov     r5, #s_XISEG                    ; dst cursor (xdata)
        mov     r6, #(s_XISEG >> 8)
00001$:
        mov     dph, r4                         ; load src, fetch one byte
        mov     dpl, r3
        clr     a
        movc    a, @a+dptr
        inc     dptr
        mov     r3, dpl
        mov     r4, dph

        mov     dph, r6                         ; load dst, store it
        mov     dpl, r5
        movx    @dptr, a
        inc     dptr
        mov     r5, dpl
        mov     r6, dph

        djnz    r1, 00001$
        djnz    r2, 00001$
00003$:
        mov     dpl, #0x00                      ; 0 = let SDCC run its normal init too
        ret
    __endasm;
    // clang-format on
}
