#pragma once

#include <stdint.h>
#include <compiler.h>

// INSCON[7:6] multiplexes two banks onto the same SFR addresses: page 0 carries P0-P4,
// Timer0/1 and FLASHCON, page 1 carries P5-P8, Timer3/4 and the USB block.
#define INSCON_PAGE_MASK 0x3Fu
#define INSCON_PAGE_1    0x40u

#define sfr_page_1() (INSCON |= INSCON_PAGE_1)
#define sfr_page_0() (INSCON &= INSCON_PAGE_MASK)

// CPU
SFR(ACC, 0xe0);
SFR(B, 0xf0);
SFR(PSW, 0xd0);
SFR(SP, 0x81);
SFR(DPL, 0x82);
SFR(DPH, 0x83);
SFR(INSCON, 0x86);
SFR(PCON, 0x87);

// INTERRUPTS
SFR(IEN0, 0xa8);
SFR(IEN1, 0xa9);
SFR(IPH0, 0xb4);

// WDT
SFR(RSTSTAT, 0xb1);

// CLOCK / POWER
SFR(CLKCON, 0xb2);
SFR(PLLCON, 0xb3);
SFR(REGCON, 0xa1);

// FLASH
SFR(FLASHCON, 0xa7);
SFR(XPAGE, 0xf7);
SFR(IB_OFFSET, 0xfb);
SFR(IB_DATA, 0xfc);
SFR(IB_CON1, 0xf2);
SFR(IB_CON2, 0xf3);
SFR(IB_CON3, 0xf4);
SFR(IB_CON4, 0xf5);
SFR(IB_CON5, 0xf6);

// GPIO, page 0
SFR(P0, 0x80);
SFR(P1, 0x90);
SFR(P2, 0xa0);
SFR(P3, 0xb0);
SFR(P4, 0xc0);
SFR(P0CR, 0xe1);
SFR(P1CR, 0xe2);
SFR(P2CR, 0xe3);
SFR(P3CR, 0xe4);
SFR(P4CR, 0xe5);
SFR(P0PCR, 0xe9);
SFR(P1PCR, 0xea);
SFR(P2PCR, 0xeb);
SFR(P3PCR, 0xec);
SFR(P4PCR, 0xed);

// GPIO, page 1
SFR(P5, 0x80);
SFR(P6, 0x90);
SFR(P7, 0xa0);
SFR(P8, 0xb0);
SFR(P5CR, 0xe1);
SFR(P6CR, 0xe2);
SFR(P7CR, 0xe3);
SFR(P8CR, 0xe4);
SFR(P5PCR, 0xe9);
SFR(P6PCR, 0xea);
SFR(P7PCR, 0xeb);

SBIT(EA, 0xa8, 7);
