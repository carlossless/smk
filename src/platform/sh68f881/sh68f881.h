#pragma once

#include <stdint.h>
#include <compiler.h>

#define _SBUF(addr) static __xdata __at(addr) volatile uint8_t

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

// USB, page 1
SFR(EP0CON, 0x98);
SFR(USBCON, 0x99);
SFR(USBIE1, 0x9a);
SFR(USBIE2, 0x9b);
SFR(USBADDR, 0x9c);
SFR(IEP0CNT, 0x9d);
SFR(IEP1CNT, 0x9e);
SFR(IEP2CNT, 0x9f);
SFR(OEP0CNT, 0xa5);
SFR(OEP1CNT, 0xa6);
SFR(OEP2CNT, 0xa7);
SFR(EP1CON, 0xc0);
SFR(EP2CON, 0xd8);
SFR(USBIF1, 0xe8);
SFR(USBIF2, 0xf8);

#define _OEP0RDY 0x01u
#define _OEP0STL 0x02u
#define _IEP0RDY 0x04u
#define _IEP0STL 0x08u
#define _OEP0DTG 0x40u
#define _IEP0DTG 0x80u

#define _USBRSTIF 0x01u
#define _SUSPIF   0x02u
#define _RESMIF   0x04u
#define _SOFIF    0x08u
#define _SETUPIF  0x10u
#define _OW       0x20u
#define _OVERIF   0x40u
#define _PUPIF    0x80u

#define _IEP0IF 0x01u
#define _IEP1IF 0x02u
#define _IEP2IF 0x04u
#define _OEP0IF 0x10u
#define _OEP1IF 0x20u
#define _OEP2IF 0x40u

#define _EUSB 0x10u

// Values the bootloader's own USB bring-up uses.
#define USBCON_ENABLE 0xC0u
#define USBIE1_INIT   0x57u
#define USBIE2_INIT   0x11u

#define EP0_BUF_SIZE 8u
#define EP1_BUF_SIZE 16u
#define EP2_BUF_SIZE 64u

_SBUF(0x0a00) EP0_OUT_BUF[EP0_BUF_SIZE];
_SBUF(0x0a08) EP0_IN_BUF[EP0_BUF_SIZE];
_SBUF(0x0a10) EP1_OUT_BUF[EP1_BUF_SIZE];
_SBUF(0x0a20) EP1_IN_BUF[EP1_BUF_SIZE];
_SBUF(0x0a30) EP1_COM_BUF[EP1_BUF_SIZE];
_SBUF(0x0a40) EP2_OUT_BUF[EP2_BUF_SIZE];
_SBUF(0x0a80) EP2_IN_BUF[EP2_BUF_SIZE];
_SBUF(0x0ac0) EP2_COM_BUF[EP2_BUF_SIZE];
