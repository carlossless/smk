#include <stdint.h>

// Initialized-xdata fixup for SDCC's broken XINIT copy on this part.
//
// SDCC's __mcs51_genXINIT copies the XINIT image into XISEG with paged
// addressing (`mov P2,#page` / `movx @r0,a`), assembling P2 at 0xA0 per the
// classic 8051 SFR map. The SH68F90A puts P2 at 0x98 (see sh68f90a.h), so the
// page is never set and the copy scatters, leaving every initialized __xdata
// variable holding power-on garbage. Only visible on a cold boot; an ISP reset
// keeps the previous run's xdata values, hiding it.
//
// SDCC calls this hook from GSINIT2, before its own initialization. Redo the
// copy here with DPTR, then return 0 so SDCC still runs its RAM clear and
// C-level initializers. Its own genXINIT still runs, but only rewrites the same
// bytes into unused xdata, and its RAM clear covers XSEG only
// (0x0000..s_XISEG-1), never the XISEG bytes written here.
//
// Only one DPTR is available, so the loop reloads it per byte: MOVC (src) and
// MOVX (dst) both need it.
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
