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

// SPI, page 0. Stock only clears these in the same block; nothing here drives SPI.
SFR(SPCON, 0xa4);
SFR(SPSTA, 0xa5);
SFR(SPDAT, 0xa6);

// CPU
SFR(ACC, 0xe0);
SFR(B, 0xf0);
SFR(PSW, 0xd0);
SFR(SP, 0x81);
SFR(DPL, 0x82);
SFR(DPH, 0x83);
SFR(DPL1, 0x84);
SFR(DPH1, 0x85);
SFR(AUXC, 0xf1);
SFR(INSCON, 0x86);
SFR(PCON, 0x87);

// INTERRUPTS
SFR(IEN0, 0xa8);
SFR(IEN1, 0xa9);
SFR(IPH0, 0xb4);

SFR(IPL0, 0xb8);
SFR(IPL1, 0xb9);
SFR(IPH1, 0xb5);
SFR(IENC, 0xba);

// Vector 0x005B is IEN1 bit 4: (0x5B - 3) / 8.
enum { _INT_TIMER2 = 5, _INT_USB = 11 };

// POWER
SFR(SUSLO, 0x8e);

// WDT
SFR(RSTSTAT, 0xb1);

// CLOCK / POWER
SFR(CLKLO, 0xbd);
SFR(CLKRC0, 0xbe);
SFR(CLKRC1, 0xbf);
SFR(CLKCON, 0xb2);
SFR(PLLCON, 0xb3);
SFR(REGCON, 0xa1);

// PWM. Three channels; nothing here uses them, the backlight is multiplexed in software.
SFR(PWM0CON, 0xc5);
SFR(PWM1CON, 0xc6);
SFR(PWM2CON, 0xc7);
SFR(PWM0PL, 0xd1);
SFR(PWM0PH, 0xd2);
SFR(PWM1PL, 0xd3);
SFR(PWM1PH, 0xd4);
SFR(PWM2PL, 0xd5);
SFR(PWM2PH, 0xd6);
SFR(PWM0DL, 0xd9);
SFR(PWM0DH, 0xda);
SFR(PWM1DL, 0xdb);
SFR(PWM1DH, 0xdc);
SFR(PWM2DL, 0xdd);
SFR(PWM2DH, 0xde);

// BASE TIMER
SFR(BTCON, 0xc1);
SFR(SEC, 0xc2);
SFR(MIN, 0xc3);

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
SFR(P8PCR, 0xec);

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

#define _EUSB 0x10u

/**
 * \name Bits from register USBCON
 * @{
 */
#define _GOSUSP (1u << 0)
#define _WKUP   (1u << 1)
#define _SW2CON (1u << 2)
#define _DMSTA  (1u << 3)
#define _DPSTA  (1u << 4)
#define _SWRST  (1u << 5)
#define _SW1CON (1u << 6)
#define _ENUSB  (1u << 7)
/**@}*/

/**
 * \name Bits from register USBIF1
 * @{
 */
#define _USBRSTIF (1u << 0)
#define _SUSPIF   (1u << 1)
#define _RESMIF   (1u << 2)
#define _SOFIF    (1u << 3)
#define _SETUPIF  (1u << 4)
#define _OW       (1u << 5)
#define _OVERIF   (1u << 6)
#define _PUPIF    (1u << 7)
/**@}*/

/**
 * \name Bits from register USBIF2
 * @{
 */
#define _IEP0IF (1u << 0)
#define _IEP1IF (1u << 1)
#define _IEP2IF (1u << 2)
#define _OEP0IF (1u << 4)
#define _OEP1IF (1u << 5)
#define _OEP2IF (1u << 6)
/**@}*/

/**
 * \name Bits from register USBIE1
 * @{
 */
#define _PBRSTIE (1u << 0)
#define _SUSPIE  (1u << 1)
#define _RESMIE  (1u << 2)
#define _SOFIA   (1u << 3)
#define _SETUPIE (1u << 4)
#define _OVERIE  (1u << 6)
#define _BOOTS   (1u << 7)
/**@}*/

/**
 * \name Bits from register USBIE2
 * @{
 */
#define _IEP0IE (1u << 0)
#define _IEP1IE (1u << 1)
#define _IEP2IE (1u << 2)
#define _OEP0IE (1u << 4)
#define _OEP1IE (1u << 5)
#define _OEP2IE (1u << 6)
/**@}*/

/**
 * \name Bits from register EP0CON
 * @{
 */
#define _OEP0RDY (1u << 0)
#define _OEP0STL (1u << 1)
#define _IEP0RDY (1u << 2)
#define _IEP0STL (1u << 3)
#define _OEP0DTG (1u << 6)
#define _IEP0DTG (1u << 7)
/**@}*/

/**
 * \name Bits from register EP1CON
 * @{
 */
#define _OEP1RDY    (1u << 0)
#define _OEP1STL    (1u << 1)
#define _IEP1RDY    (1u << 2)
#define _IEP1STL    (1u << 3)
#define _OEP1BUFSEL (1u << 4)
#define _IEP1BUFSEL (1u << 5)
#define _OEP1DTG    (1u << 6)
#define _IEP1DTG    (1u << 7)
/**@}*/

/**
 * \name Bits from register EP2CON
 * @{
 */
#define _OEP2RDY    (1u << 0)
#define _OEP2STL    (1u << 1)
#define _IEP2RDY    (1u << 2)
#define _IEP2STL    (1u << 3)
#define _OEP2BUFSEL (1u << 4)
#define _IEP2BUFSEL (1u << 5)
#define _OEP2DTG    (1u << 6)
#define _IEP2DTG    (1u << 7)
/**@}*/

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
_SBUF(0x0a30) EP2_OUT_BUF[EP2_BUF_SIZE];
_SBUF(0x0a70) EP2_IN_BUF[EP2_BUF_SIZE];

// TIMER 2, page 0
SFR(T2MOD, 0xc9);
SFR(T2CON, 0xc8);
SFR(RCAP2L, 0xca);
SFR(RCAP2H, 0xcb);
SFR(TL2, 0xcc);
SFR(TH2, 0xcd);

SBIT(TR2, 0xc8, 2);
SBIT(TF2, 0xc8, 7);
SBIT(ET2, 0xa8, 5);

// TIMER 0, page 0 -- used to measure the core clock against the delay loop.
SFR(TCON, 0x88);
SFR(TMOD, 0x89);
SFR(TL0, 0x8a);
SFR(TH0, 0x8c);

SBIT(TR0, 0x88, 4);
SBIT(TF0, 0x88, 5);
