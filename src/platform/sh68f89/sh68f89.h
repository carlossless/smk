#pragma once

#include <stdint.h>
#include <compiler.h>

#define _SBUF(addr) static __xdata __at(addr) volatile uint8_t

// bit 7 is cleared alongside BKS0: the ISP bootloader selects page 0 with `anl INSCON,#0x3f`.
#define INSCON_PAGE_MASK (uint8_t)~(_BKS0 | 0x80u)

#define sfr_page_1() (INSCON |= _BKS0)
#define sfr_page_0() (INSCON &= INSCON_PAGE_MASK)

// CPU, both pages
SFR(ACC, 0xe0);
SFR(B, 0xf0);
SFR(AUXC, 0xf1);
SFR(PSW, 0xd0);
SFR(SP, 0x81);
SFR(DPL, 0x82);
SFR(DPH, 0x83);
SFR(DPL1, 0x84);
SFR(DPH1, 0x85);
SFR(INSCON, 0x86);

// POWER, both pages
SFR(PCON, 0x87);
SFR(SUSLO, 0x8e);

// FLASH, page 0
SFR(IB_CON1, 0xf2);
SFR(IB_CON2, 0xf3);
SFR(IB_CON3, 0xf4);
SFR(IB_CON4, 0xf5);
SFR(IB_CON5, 0xf6);
SFR(IB_OFFSET, 0xfb);
SFR(IB_DATA, 0xfc);
SFR(XPAGE, 0xf7);
SFR(FLASHCON, 0xa7);

// ISP, page 0
SFR(ISPLO, 0xa5);
SFR(ISPCON, 0xa6);

// WDT, page 0
SFR(RSTSTAT, 0xb1);

#define WATCHDOG_PERIOD 0x02 // _WDT1

#define ISP_ENTRY 0xff00
#define ISP_KEY_B 0xa5
#define ISP_KEY_A 0xea

// SYSTEM CLOCK, page 0
SFR(CLKCON, 0xb2);
SFR(PLLCON, 0xbc);
SFR(CLKLO, 0xbd);
SFR(CLKRC0, 0xbe);
SFR(CLKRC1, 0xbf);

// REGULATOR, page 0
SFR(REGCON, 0x8f);

#define REGCON_ENABLE (uint8_t)(_REGEN)

// LOW POWER DETECT, page 0
SFR(LPDCON, 0xb3);

// INTERRUPTS, both pages for IEN0/IEN1/IPH/IPL, page 0 for the flags
SFR(IEN0, 0xa8);
SFR(IEN1, 0xa9);
SFR(IPH0, 0xb4);
SFR(IPL0, 0xb8);
SFR(IPH1, 0xb5);
SFR(IPL1, 0xb9);
SFR(EXF0, 0x88);
SFR(EXF1, 0xe8);
SFR(EXCON, 0xc2);
SFR(IENC, 0xc3);

// TIMER 2, page 0
SFR(T2CON, 0xc8);
SFR(T2MOD, 0xc9);
SFR(RCAP2L, 0xca);
SFR(RCAP2H, 0xcb);
SFR(TL2, 0xcc);
SFR(TH2, 0xcd);

// TIMER 3, page 0
SFR(T3CON, 0xbb);
SFR(TL3, 0x89);
SFR(TH3, 0x8a);

// EUART0, page 0
SFR(SCON, 0x98);
SFR(SBUF, 0x99);
SFR(SADDR, 0x9a);
SFR(SADEN, 0x9b);
SFR(SBRTL, 0x9c);
SFR(SBRTH, 0x9d);
SFR(SFINE, 0x9e);

// EUART1, page 0
SFR(SCON1, 0xd8);
SFR(SBUF1, 0xd9);
SFR(SADDR1, 0xda);
SFR(SADEN1, 0xdb);
SFR(SBRTL1, 0xdc);
SFR(SBRTH1, 0xdd);
SFR(SFINE1, 0xde);
SFR(PCON1, 0xd1);

// SPI, page 0
SFR(SPSTA, 0xf8);
SFR(SPCON, 0xf9);
SFR(SPDAT, 0xfa);

// DAC, page 0
SFR(DACCON0, 0x8b);
SFR(DACCON1, 0x9f);
SFR(DACCAL, 0xa4);
SFR(DACL, 0x8c);
SFR(DACH, 0x8d);

// OP, page 0
SFR(OPCON, 0xa1);
SFR(OPIOS, 0xa2);

// LCD, page 0
SFR(DISPCON, 0xc4);
SFR(DISPCON1, 0xc5);
SFR(P1SS, 0xc6);
SFR(P2SS, 0xc7);
SFR(P3SS, 0xce);
SFR(P4SS, 0xcf);
SFR(P5SS, 0xdf);
SFR(P6SS, 0xe7);
SFR(P7SS, 0xef);

// ADC, page 1
SFR(ADCON1, 0x91);
SFR(ADCON2, 0x8d);
SFR(ADT, 0x89);
SFR(SCHCON1, 0x8a);
SFR(SCHCON2, 0x8b);
SFR(SCHCON3, 0x8c);
SFR(ADCL, 0x92);
SFR(ADCH, 0x93);
SFR(ADCGTL, 0x94);
SFR(ADCGTH, 0x95);
SFR(ADCLTL, 0x96);
SFR(ADCLTH, 0x97);

// TWI, page 1
SFR(TWICON, 0xc0);
SFR(TWISTA, 0xc1);
SFR(TWIBR, 0xc2);
SFR(TWIADR, 0xc3);
SFR(TWIDAT, 0xc4);
SFR(TWIAMR, 0xc5);
// the vendor sources disagree on these two; the SFR map and the vendor header both put
// TWTOUT at 0xc6, so the CNT bits below may in fact belong to the other address.
SFR(TWTOUT, 0xc6);
SFR(TWTFREE, 0xc7);

// USB, page 1
SFR(USBCON, 0xb1);
SFR(USBIF1, 0xb0);
SFR(USBIF2, 0x88);
SFR(USBIE1, 0xb2);
SFR(USBIE2, 0xb3);
SFR(USBADDR, 0xbe);
SFR(EP0CON, 0xbf);
SFR(EP1CON, 0xaa);
SFR(EP2CON, 0xab);
SFR(IEP0CNT, 0xac);
SFR(IEP1CNT, 0xad);
SFR(IEP2CNT, 0xae);
SFR(OEP0CNT, 0xaf);
SFR(OEP1CNT, 0xb6);
SFR(OEP2CNT, 0xb7);

// PCA0, page 1
SFR(P0CF, 0x98);
SFR(P0CMD, 0x99);
SFR(P0CPM0, 0x9a);
SFR(P0CPM1, 0x9b);
SFR(P0TOPL, 0x9e);
SFR(P0TOPH, 0x9f);
SFR(P0CPL0, 0xa4);
SFR(P0CPH0, 0xa5);
SFR(P0CPL1, 0xa6);
SFR(P0CPH1, 0xa7);

// PCA1, page 1
SFR(P1CF, 0xc8);
SFR(P1CMD, 0xc9);
SFR(P1CPM0, 0xca);
SFR(P1CPM1, 0xcb);
SFR(P1CPM2, 0xd1);
SFR(P1TOPL, 0xce);
SFR(P1TOPH, 0xcf);
SFR(P1CPL0, 0xd2);
SFR(P1CPH0, 0xd3);
SFR(P1CPL1, 0xd4);
SFR(P1CPH1, 0xd5);
SFR(P1CPL2, 0xd6);
SFR(P1CPH2, 0xd7);

// PCA2, page 1
SFR(P2CF, 0xe8);
SFR(P2CMD, 0xe9);
SFR(P2CPM0, 0xea);
SFR(P2CPM1, 0xeb);
SFR(P2TOPL, 0xee);
SFR(P2TOPH, 0xef);
SFR(P2CPL0, 0xe4);
SFR(P2CPH0, 0xe5);
SFR(P2CPL1, 0xe6);
SFR(P2CPH1, 0xe7);

// PCA3, page 1
SFR(P3CF, 0xf8);
SFR(P3CMD, 0xf9);
SFR(P3CPM0, 0xfa);
SFR(P3CPM1, 0xf2);
SFR(P3TOPL, 0xfd);
SFR(P3TOPH, 0xfe);
SFR(P3CPL0, 0xf3);
SFR(P3CPH0, 0xf4);
SFR(P3CPL1, 0xf5);
SFR(P3CPH1, 0xf6);

// PCA common, page 1
SFR(PCACON, 0xd8);
// the SFR map leaves DCH-DFH blank on page 1 and gives them to EUART1 on page 0, but
// table 7.9 and the vendor header call them PCA registers, so they are taken as page 1.
SFR(P0FORCE, 0xdc);
SFR(P1FORCE, 0xdd);
SFR(P2FORCE, 0xde);
SFR(P3FORCE, 0xdf);

// PORT, page 0
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

// PORT, page 1
SFR(P5, 0x80);
SFR(P6, 0x90);
SFR(P7, 0xa0);

SFR(P5CR, 0xd9);
SFR(P6CR, 0xda);
SFR(P7CR, 0xdb);

SFR(P5PCR, 0xe1);
SFR(P6PCR, 0xe2);
SFR(P7PCR, 0xe3);

SFR(P6OS, 0xa1);

// SFR 0xff is (Reserved) on page 0 and (Reserved for Testing) on page 1. Writing 0x80
// forces ISP/debug mode; that is a firmware-derived finding, not a documented register.
SFR(ISPDBG, 0xff);

// bits, page 0

// P0
SBIT(P0_0, 0x80, 0);
SBIT(P0_1, 0x80, 1);
SBIT(P0_2, 0x80, 2);
SBIT(P0_3, 0x80, 3);
SBIT(P0_4, 0x80, 4);
SBIT(P0_5, 0x80, 5);
SBIT(P0_6, 0x80, 6);
SBIT(P0_7, 0x80, 7);

// P1
SBIT(P1_0, 0x90, 0);
SBIT(P1_1, 0x90, 1);
SBIT(P1_2, 0x90, 2);
SBIT(P1_3, 0x90, 3);
SBIT(P1_4, 0x90, 4);
SBIT(P1_5, 0x90, 5);
SBIT(P1_6, 0x90, 6);
SBIT(P1_7, 0x90, 7);

// P2
SBIT(P2_0, 0xa0, 0);
SBIT(P2_1, 0xa0, 1);
SBIT(P2_2, 0xa0, 2);
SBIT(P2_3, 0xa0, 3);
SBIT(P2_4, 0xa0, 4);
SBIT(P2_5, 0xa0, 5);
SBIT(P2_6, 0xa0, 6);
SBIT(P2_7, 0xa0, 7);

// P3
SBIT(P3_0, 0xb0, 0);
SBIT(P3_1, 0xb0, 1);
SBIT(P3_2, 0xb0, 2);
SBIT(P3_3, 0xb0, 3);
SBIT(P3_4, 0xb0, 4);
SBIT(P3_5, 0xb0, 5);
SBIT(P3_6, 0xb0, 6);
SBIT(P3_7, 0xb0, 7);

// P4
SBIT(P4_0, 0xc0, 0);
SBIT(P4_1, 0xc0, 1);
SBIT(P4_2, 0xc0, 2);
SBIT(P4_3, 0xc0, 3);
SBIT(P4_4, 0xc0, 4);
SBIT(P4_5, 0xc0, 5);
SBIT(P4_6, 0xc0, 6);
SBIT(P4_7, 0xc0, 7);

// PSW
SBIT(CY, 0xd0, 7);
SBIT(AC, 0xd0, 6);
SBIT(F0, 0xd0, 5);
SBIT(RS1, 0xd0, 4);
SBIT(RS0, 0xd0, 3);
SBIT(OV, 0xd0, 2);
SBIT(F1, 0xd0, 1);
SBIT(P, 0xd0, 0);

// IEN0
SBIT(EA, 0xa8, 7);
SBIT(EADC, 0xa8, 6);
SBIT(ET2, 0xa8, 5);
SBIT(ES0, 0xa8, 4);
SBIT(EPCA3, 0xa8, 3);
SBIT(EPCA2, 0xa8, 2);
SBIT(EPCA1, 0xa8, 1);
SBIT(EPCA0, 0xa8, 0);

// IPL0
SBIT(PADCL, 0xb8, 6);
SBIT(PT2L, 0xb8, 5);
SBIT(PS0L, 0xb8, 4);
SBIT(PPCA3L, 0xb8, 3);
SBIT(PPCA2L, 0xb8, 2);
SBIT(PPCA1L, 0xb8, 1);
SBIT(PPCA0L, 0xb8, 0);

// EXF0
SBIT(IT41, 0x88, 7);
SBIT(IT40, 0x88, 6);
SBIT(IT31, 0x88, 5);
SBIT(IT30, 0x88, 4);
SBIT(IT21, 0x88, 3);
SBIT(IT20, 0x88, 2);
SBIT(IE3, 0x88, 1);
SBIT(IE2, 0x88, 0);

// EXF1
SBIT(IF47, 0xe8, 7);
SBIT(IF46, 0xe8, 6);
SBIT(IF45, 0xe8, 5);
SBIT(IF44, 0xe8, 4);
SBIT(IF43, 0xe8, 3);
SBIT(IF42, 0xe8, 2);
SBIT(IF41, 0xe8, 1);
SBIT(IF40, 0xe8, 0);

// T2CON
SBIT(TF2, 0xc8, 7);
SBIT(EXF2, 0xc8, 6);
SBIT(EXEN2, 0xc8, 3);
SBIT(TR2, 0xc8, 2);
SBIT(C_T2, 0xc8, 1);
SBIT(CP_RL2, 0xc8, 0);

// SCON
SBIT(SM0_FE, 0x98, 7);
SBIT(SM1_RXOV, 0x98, 6);
SBIT(SM2_TXCOL, 0x98, 5);
SBIT(REN, 0x98, 4);
SBIT(TB8, 0x98, 3);
SBIT(RB8, 0x98, 2);
SBIT(TI, 0x98, 1);
SBIT(RI, 0x98, 0);

// SCON1
SBIT(SM10_FE1, 0xd8, 7);
SBIT(SM11_RXOV1, 0xd8, 6);
SBIT(SM12_TXCOL1, 0xd8, 5);
SBIT(REN1, 0xd8, 4);
SBIT(TB81, 0xd8, 3);
SBIT(RB81, 0xd8, 2);
SBIT(TI1, 0xd8, 1);
SBIT(RI1, 0xd8, 0);

// SPSTA
SBIT(SPEN, 0xf8, 7);
SBIT(SPIF, 0xf8, 6);
SBIT(MODF, 0xf8, 5);
SBIT(WCOL, 0xf8, 4);
SBIT(RXOV, 0xf8, 3);

// bits, page 1

// P5
SBIT(P5_0, 0x80, 0);
SBIT(P5_1, 0x80, 1);
SBIT(P5_2, 0x80, 2);
SBIT(P5_3, 0x80, 3);
SBIT(P5_4, 0x80, 4);
SBIT(P5_5, 0x80, 5);
SBIT(P5_6, 0x80, 6);
SBIT(P5_7, 0x80, 7);

// P6
SBIT(P6_0, 0x90, 0);
SBIT(P6_1, 0x90, 1);
SBIT(P6_2, 0x90, 2);
SBIT(P6_3, 0x90, 3);
SBIT(P6_4, 0x90, 4);
SBIT(P6_5, 0x90, 5);
SBIT(P6_6, 0x90, 6);
SBIT(P6_7, 0x90, 7);

// P7
SBIT(P7_0, 0xa0, 0);
SBIT(P7_1, 0xa0, 1);
SBIT(P7_2, 0xa0, 2);
SBIT(P7_3, 0xa0, 3);
SBIT(P7_4, 0xa0, 4);

// USBIF1
SBIT(PUPIF, 0xb0, 7);
SBIT(OVERIF, 0xb0, 6);
SBIT(OW, 0xb0, 5);
SBIT(SETUPIF, 0xb0, 4);
SBIT(SOFIF, 0xb0, 3);
SBIT(RESMIF, 0xb0, 2);
SBIT(SUSPIF, 0xb0, 1);
SBIT(USBRSTIF, 0xb0, 0);

// USBIF2
SBIT(OEP2IF, 0x88, 6);
SBIT(OEP1IF, 0x88, 5);
SBIT(OEP0IF, 0x88, 4);
SBIT(IEP2IF, 0x88, 2);
SBIT(IEP1IF, 0x88, 1);
SBIT(IEP0IF, 0x88, 0);

// TWICON
SBIT(TOUT, 0xc0, 7);
SBIT(ENTWI, 0xc0, 6);
SBIT(STA, 0xc0, 5);
SBIT(STO, 0xc0, 4);
SBIT(TWINT, 0xc0, 3);
SBIT(AA, 0xc0, 2);
SBIT(TFREE, 0xc0, 1);
SBIT(EFREE, 0xc0, 0);

// PCACON
SBIT(PR3, 0xd8, 3);
SBIT(PR2, 0xd8, 2);
SBIT(PR1, 0xd8, 1);
SBIT(PR0, 0xd8, 0);

// P0CF
SBIT(CF0, 0x98, 7);
SBIT(P0CCF1, 0x98, 1);
SBIT(P0CCF0, 0x98, 0);

// P1CF
SBIT(CF1, 0xc8, 7);
SBIT(P1CCF2, 0xc8, 2);
SBIT(P1CCF1, 0xc8, 1);
SBIT(P1CCF0, 0xc8, 0);

// P2CF
SBIT(CF2, 0xe8, 7);
SBIT(P2CCF1, 0xe8, 1);
SBIT(P2CCF0, 0xe8, 0);

// P3CF
SBIT(CF3, 0xf8, 7);
SBIT(P3CCF1, 0xf8, 1);
SBIT(P3CCF0, 0xf8, 0);

// EP0CON, EP1CON and EP2CON sit at 0xbf, 0xaa and 0xab, none of them bit addressable,
// so every endpoint bit below is a mask for a read-modify-write.

/**
 * \name Bits from register INSCON
 * @{
 */
#define _DPS  (1u << 0)
#define _MUL  (1u << 2)
#define _DIV  (1u << 3)
#define _BKS0 (1u << 6)
/**@}*/

/**
 * \name Bits from register PCON
 * @{
 */
#define _IDL   (1u << 0)
#define _PD    (1u << 1)
#define _GF0   (1u << 2)
#define _GF1   (1u << 3)
#define _SSTAT (1u << 6)
#define _SMOD  (1u << 7)
/**@}*/

/**
 * \name Bits from register RSTSTAT
 * @{
 */
#define _WDT0 (1u << 0)
#define _WDT1 (1u << 1)
#define _WDT2 (1u << 2)
#define _CLRF (1u << 3)
#define _LVRF (1u << 4)
#define _PORF (1u << 5)
#define _WDOF (1u << 7)
/**@}*/

/**
 * \name Bits from register CLKCON
 * @{
 */
#define _FS       (1u << 2)
#define _OSC2ON   (1u << 3)
#define _SCMIF    (1u << 4)
#define _CLKS0    (1u << 5)
#define _CLKS1    (1u << 6)
#define _SPDUP32K (1u << 7)
/**@}*/

/**
 * \name Bits from register PLLCON
 * @{
 */
#define _PLLFS  (1u << 0)
#define _PLLON  (1u << 1)
#define _PLLSTA (1u << 2)
/**@}*/

/**
 * \name Bits from register CLKLO
 * @{
 */
#define _CLKLO0  (1u << 0)
#define _CLKLO1  (1u << 1)
#define _CLKLO2  (1u << 2)
#define _CLKLO3  (1u << 3)
#define _CLKRCEN (1u << 7)
/**@}*/

/**
 * \name Bits from register LPDCON
 * @{
 */
#define _LPDS0 (1u << 0)
#define _LPDS1 (1u << 1)
#define _LPDS2 (1u << 2)
#define _LPDS3 (1u << 3)
#define _LPDIF (1u << 4)
#define _LPDMD (1u << 5)
#define _LPDEN (1u << 7)
/**@}*/

/**
 * \name Bits from register REGCON
 * @{
 */
#define _REGEN (1u << 0)
/**@}*/

/**
 * \name Bits from register FLASHCON
 * @{
 */
#define _FAC (1u << 0)
/**@}*/

/**
 * \name Bits from register IEN0
 * @{
 */
#define _EPCA0 (1u << 0)
#define _EPCA1 (1u << 1)
#define _EPCA2 (1u << 2)
#define _EPCA3 (1u << 3)
#define _ES0   (1u << 4)
#define _ET2   (1u << 5)
#define _EADC  (1u << 6)
#define _EA    (1u << 7)
/**@}*/

/**
 * \name Bits from register IEN1
 * @{
 */
#define _ESPI      (1u << 0)
#define _EX2_EDAC  (1u << 1)
#define _EX3       (1u << 2)
#define _EX4       (1u << 3)
#define _ET3       (1u << 4)
#define _EUSB_ETWI (1u << 5)
#define _ES1       (1u << 6)
#define _ESCM_ELPD (1u << 7)
/**@}*/

/**
 * \name Bits from register IPH0
 * @{
 */
#define _PPCA0H (1u << 0)
#define _PPCA1H (1u << 1)
#define _PPCA2H (1u << 2)
#define _PPCA3H (1u << 3)
#define _PS0H   (1u << 4)
#define _PT2H   (1u << 5)
#define _PADCH  (1u << 6)
/**@}*/

/**
 * \name Bits from register IPL0
 * @{
 */
#define _PPCA0L (1u << 0)
#define _PPCA1L (1u << 1)
#define _PPCA2L (1u << 2)
#define _PPCA3L (1u << 3)
#define _PS0L   (1u << 4)
#define _PT2L   (1u << 5)
#define _PADCL  (1u << 6)
/**@}*/

/**
 * \name Bits from register IPH1
 * @{
 */
#define _PSPIH     (1u << 0)
#define _PX2DACH   (1u << 1)
#define _PX3H      (1u << 2)
#define _PX4H      (1u << 3)
#define _PT3H      (1u << 4)
#define _PUSB_TWIH (1u << 5)
#define _PS1H      (1u << 6)
#define _PSCM_LPDH (1u << 7)
/**@}*/

/**
 * \name Bits from register IPL1
 * @{
 */
#define _PSPIL     (1u << 0)
#define _PX2DACL   (1u << 1)
#define _PX3L      (1u << 2)
#define _PX4L      (1u << 3)
#define _PT3L      (1u << 4)
#define _PUSB_TWIL (1u << 5)
#define _PS1L      (1u << 6)
#define _PSCM_LPDL (1u << 7)
/**@}*/

/**
 * \name Bits from register IENC
 * @{
 */
#define _EXS40 (1u << 0)
#define _EXS41 (1u << 1)
#define _EXS42 (1u << 2)
#define _EXS43 (1u << 3)
#define _EXS44 (1u << 4)
#define _EXS45 (1u << 5)
#define _EXS46 (1u << 6)
#define _EXS47 (1u << 7)
/**@}*/

/**
 * \name Bits from register EXF0
 * @{
 */
#define _IE2  (1u << 0)
#define _IE3  (1u << 1)
#define _IT20 (1u << 2)
#define _IT21 (1u << 3)
#define _IT30 (1u << 4)
#define _IT31 (1u << 5)
#define _IT40 (1u << 6)
#define _IT41 (1u << 7)
/**@}*/

/**
 * \name Bits from register EXF1
 * @{
 */
#define _IF40 (1u << 0)
#define _IF41 (1u << 1)
#define _IF42 (1u << 2)
#define _IF43 (1u << 3)
#define _IF44 (1u << 4)
#define _IF45 (1u << 5)
#define _IF46 (1u << 6)
#define _IF47 (1u << 7)
/**@}*/

/**
 * \name Bits from register EXCON
 * @{
 */
#define _IP0     (1u << 0)
#define _IP1     (1u << 1)
#define _EXTFS0  (1u << 2)
#define _EXTFS1  (1u << 3)
#define _I4P0    (1u << 4)
#define _I4P1    (1u << 5)
#define _EXT4FS0 (1u << 6)
#define _EXT4FS1 (1u << 7)
/**@}*/

/**
 * \name Bits from register T2CON
 * @{
 */
#define _CP_RL2 (1u << 0)
#define _C_T2   (1u << 1)
#define _TR2    (1u << 2)
#define _EXEN2  (1u << 3)
#define _EXF2   (1u << 6)
#define _TF2    (1u << 7)
/**@}*/

/**
 * \name Bits from register T2MOD
 * @{
 */
#define _DCEN   (1u << 0)
#define _T2OE   (1u << 1)
#define _TCLKP2 (1u << 7)
/**@}*/

/**
 * \name Bits from register T3CON
 * @{
 */
#define _T3CLKS0 (1u << 0)
#define _T3CLKS1 (1u << 1)
#define _TR3     (1u << 2)
#define _T3PS0   (1u << 4)
#define _T3PS1   (1u << 5)
#define _TF3     (1u << 7)
/**@}*/

/**
 * \name Bits from register SCON
 * @{
 */
#define _RI        (1u << 0)
#define _TI        (1u << 1)
#define _RB8       (1u << 2)
#define _TB8       (1u << 3)
#define _REN       (1u << 4)
#define _SM2_TXCOL (1u << 5)
#define _SM1_RXOV  (1u << 6)
#define _SM0_FE    (1u << 7)
/**@}*/

/**
 * \name Bits from register SBRTL
 * @{
 */
#define _SBRTEN (1u << 7)
/**@}*/

/**
 * \name Bits from register SCON1
 * @{
 */
#define _RI1         (1u << 0)
#define _TI1         (1u << 1)
#define _RB81        (1u << 2)
#define _TB81        (1u << 3)
#define _REN1        (1u << 4)
#define _SM12_TXCOL1 (1u << 5)
#define _SM11_RXOV1  (1u << 6)
#define _SM10_FE1    (1u << 7)
/**@}*/

/**
 * \name Bits from register SBRTL1
 * @{
 */
#define _SBRTEN1 (1u << 7)
/**@}*/

/**
 * \name Bits from register PCON1
 * @{
 */
#define _SSTAT1 (1u << 6)
#define _SMOD1  (1u << 7)
/**@}*/

/**
 * \name Bits from register SPCON
 * @{
 */
#define _SPR0  (1u << 0)
#define _SPR1  (1u << 1)
#define _SPR2  (1u << 2)
#define _SSDIS (1u << 3)
#define _CPOL  (1u << 4)
#define _CPHA  (1u << 5)
#define _MSTR  (1u << 6)
#define _DIR   (1u << 7)
/**@}*/

/**
 * \name Bits from register SPSTA
 * @{
 */
#define _RXOV (1u << 3)
#define _WCOL (1u << 4)
#define _MODF (1u << 5)
#define _SPIF (1u << 6)
#define _SPEN (1u << 7)
/**@}*/

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
#define _SOFIE   (1u << 3)
#define _SETUPIE (1u << 4)
#define _OVERIE  (1u << 6)
#define _PUPIE   (1u << 7)
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
#define _OEP1RDY (1u << 0)
#define _OEP1STL (1u << 1)
#define _IEP1RDY (1u << 2)
#define _IEP1STL (1u << 3)
#define _OEP1DTG (1u << 6)
#define _IEP1DTG (1u << 7)
/**@}*/

/**
 * \name Bits from register EP2CON
 * @{
 */
#define _OEP2RDY (1u << 0)
#define _OEP2STL (1u << 1)
#define _IEP2RDY (1u << 2)
#define _IEP2STL (1u << 3)
#define _OEP2DTG (1u << 6)
#define _IEP2DTG (1u << 7)
/**@}*/

/**
 * \name Bits from register TWICON
 * @{
 */
#define _EFREE (1u << 0)
#define _TFREE (1u << 1)
#define _AA    (1u << 2)
#define _TWINT (1u << 3)
#define _STO   (1u << 4)
#define _STA   (1u << 5)
#define _ENTWI (1u << 6)
#define _TOUT  (1u << 7)
/**@}*/

/**
 * \name Bits from register TWISTA
 * @{
 */
#define _ETOT (1u << 0)
#define _CR0  (1u << 1)
#define _CR1  (1u << 2)
/**@}*/

/**
 * \name Bits from register TWIADR
 * @{
 */
#define _GC (1u << 0)
/**@}*/

/**
 * \name Bits from register TWIAMR
 * @{
 */
#define _CTRTOUT (1u << 0)
/**@}*/

/**
 * \name Bits from register TWTFREE
 * @{
 */
#define _CNT0 (1u << 6)
#define _CNT1 (1u << 7)
/**@}*/

/**
 * \name Bits from register PCACON
 * @{
 */
#define _PR0 (1u << 0)
#define _PR1 (1u << 1)
#define _PR2 (1u << 2)
#define _PR3 (1u << 3)
/**@}*/

/**
 * \name Bits from register P0CF
 * @{
 */
#define _P0CCF0 (1u << 0)
#define _P0CCF1 (1u << 1)
#define _CF0    (1u << 7)
/**@}*/

/**
 * \name Bits from register P0CMD
 * @{
 */
#define _P0CPS0 (1u << 0)
#define _P0CPS1 (1u << 1)
#define _P0CPS2 (1u << 2)
#define _P0SDEN (1u << 6)
#define _ECF0   (1u << 7)
/**@}*/

/**
 * \name Bits from register P1CF
 * @{
 */
#define _P1CCF0 (1u << 0)
#define _P1CCF1 (1u << 1)
#define _P1CCF2 (1u << 2)
#define _CF1    (1u << 7)
/**@}*/

/**
 * \name Bits from register P1CMD
 * @{
 */
#define _P1CPS0 (1u << 0)
#define _P1CPS1 (1u << 1)
#define _P1CPS2 (1u << 2)
#define _P1SDEN (1u << 6)
#define _ECF1   (1u << 7)
/**@}*/

/**
 * \name Bits from register P2CF
 * @{
 */
#define _P2CCF0 (1u << 0)
#define _P2CCF1 (1u << 1)
#define _CF2    (1u << 7)
/**@}*/

/**
 * \name Bits from register P2CMD
 * @{
 */
#define _P2CPS0 (1u << 0)
#define _P2CPS1 (1u << 1)
#define _P2CPS2 (1u << 2)
#define _P2SDEN (1u << 6)
#define _ECF2   (1u << 7)
/**@}*/

/**
 * \name Bits from register P3CF
 * @{
 */
#define _P3CCF0 (1u << 0)
#define _P3CCF1 (1u << 1)
#define _CF3    (1u << 7)
/**@}*/

/**
 * \name Bits from register P3CMD
 * @{
 */
#define _P3CPS0 (1u << 0)
#define _P3CPS1 (1u << 1)
#define _P3CPS2 (1u << 2)
#define _P3SDEN (1u << 6)
#define _ECF3   (1u << 7)
/**@}*/

/**
 * \name Bits from register P0CPM0
 * @{
 */
#define _P0ECCF0 (1u << 0)
#define _P0MAT0  (1u << 1)
#define _P0TCP0  (1u << 2)
#define _P0ECOM0 (1u << 3)
#define _P0FSN0  (1u << 4)
#define _P0FSP0  (1u << 5)
#define _P0SMN0  (1u << 6)
#define _P0SMP0  (1u << 7)
/**@}*/

/**
 * \name Bits from register P0CPM1
 * @{
 */
#define _P0ECCF1 (1u << 0)
#define _P0MAT1  (1u << 1)
#define _P0TCP1  (1u << 2)
#define _P0ECOM1 (1u << 3)
#define _P0FSN1  (1u << 4)
#define _P0FSP1  (1u << 5)
#define _P0SMN1  (1u << 6)
#define _P0SMP1  (1u << 7)
/**@}*/

/**
 * \name Bits from register P1CPM0
 * @{
 */
#define _P1ECCF0 (1u << 0)
#define _P1MAT0  (1u << 1)
#define _P1TCP0  (1u << 2)
#define _P1ECOM0 (1u << 3)
#define _P1FSN0  (1u << 4)
#define _P1FSP0  (1u << 5)
#define _P1SMN0  (1u << 6)
#define _P1SMP0  (1u << 7)
/**@}*/

/**
 * \name Bits from register P1CPM1
 * @{
 */
#define _P1ECCF1 (1u << 0)
#define _P1MAT1  (1u << 1)
#define _P1TCP1  (1u << 2)
#define _P1ECOM1 (1u << 3)
#define _P1FSN1  (1u << 4)
#define _P1FSP1  (1u << 5)
#define _P1SMN1  (1u << 6)
#define _P1SMP1  (1u << 7)
/**@}*/

/**
 * \name Bits from register P1CPM2
 * @{
 */
#define _P1ECCF2 (1u << 0)
#define _P1MAT2  (1u << 1)
#define _P1TCP2  (1u << 2)
#define _P1ECOM2 (1u << 3)
#define _P1FSN2  (1u << 4)
#define _P1FSP2  (1u << 5)
#define _P1SMN2  (1u << 6)
#define _P1SMP2  (1u << 7)
/**@}*/

/**
 * \name Bits from register P2CPM0
 * @{
 */
#define _P2ECCF0 (1u << 0)
#define _P2MAT0  (1u << 1)
#define _P2TCP0  (1u << 2)
#define _P2ECOM0 (1u << 3)
#define _P2FSN0  (1u << 4)
#define _P2FSP0  (1u << 5)
#define _P2SMN0  (1u << 6)
#define _P2SMP0  (1u << 7)
/**@}*/

/**
 * \name Bits from register P2CPM1
 * @{
 */
#define _P2ECCF1 (1u << 0)
#define _P2MAT1  (1u << 1)
#define _P2TCP1  (1u << 2)
#define _P2ECOM1 (1u << 3)
#define _P2FSN1  (1u << 4)
#define _P2FSP1  (1u << 5)
#define _P2SMN1  (1u << 6)
#define _P2SMP1  (1u << 7)
/**@}*/

/**
 * \name Bits from register P3CPM0
 * @{
 */
#define _P3ECCF0 (1u << 0)
#define _P3MAT0  (1u << 1)
#define _P3TCP0  (1u << 2)
#define _P3ECOM0 (1u << 3)
#define _P3FSN0  (1u << 4)
#define _P3FSP0  (1u << 5)
#define _P3SMN0  (1u << 6)
#define _P3SMP0  (1u << 7)
/**@}*/

/**
 * \name Bits from register P3CPM1
 * @{
 */
#define _P3ECCF1 (1u << 0)
#define _P3MAT1  (1u << 1)
#define _P3TCP1  (1u << 2)
#define _P3ECOM1 (1u << 3)
#define _P3FSN1  (1u << 4)
#define _P3FSP1  (1u << 5)
#define _P3SMN1  (1u << 6)
#define _P3SMP1  (1u << 7)
/**@}*/

/**
 * \name Bits from register P0FORCE
 * @{
 */
#define _P0FCO0 (1u << 0)
#define _P0FCO1 (1u << 1)
#define _P0OSC0 (1u << 4)
#define _P0OSC1 (1u << 5)
/**@}*/

/**
 * \name Bits from register P1FORCE
 * @{
 */
#define _P1FCO0 (1u << 0)
#define _P1FCO1 (1u << 1)
#define _P1FCO2 (1u << 2)
#define _P1OSC0 (1u << 4)
#define _P1OSC1 (1u << 5)
#define _P1OSC2 (1u << 6)
/**@}*/

/**
 * \name Bits from register P2FORCE
 * @{
 */
#define _P2FCO0 (1u << 0)
#define _P2FCO1 (1u << 1)
#define _P2OSC0 (1u << 4)
#define _P2OSC1 (1u << 5)
/**@}*/

/**
 * \name Bits from register P3FORCE
 * @{
 */
#define _P3FCO0 (1u << 0)
#define _P3FCO1 (1u << 1)
#define _P3OSC0 (1u << 4)
#define _P3OSC1 (1u << 5)
/**@}*/

/**
 * \name Bits from register DISPCON
 * @{
 */
#define _VOL0    (1u << 0)
#define _VOL1    (1u << 1)
#define _VOL2    (1u << 2)
#define _VOL3    (1u << 3)
#define _DUTY0   (1u << 4)
#define _DUTY1   (1u << 5)
#define _DISPON  (1u << 6)
#define _DISPSEL (1u << 7)
/**@}*/

/**
 * \name Bits from register DISPCON1
 * @{
 */
#define _MOD0   (1u << 0)
#define _MOD1   (1u << 1)
#define _FCCTL0 (1u << 2)
#define _FCCTL1 (1u << 3)
#define _RLCD   (1u << 4)
/**@}*/

/**
 * \name Bits from register OPCON
 * @{
 */
#define _OPNSEL0 (1u << 0)
#define _OPNSEL1 (1u << 1)
#define _OPPSEL0 (1u << 2)
#define _OPPSEL1 (1u << 3)
#define _OPEN    (1u << 7)
/**@}*/

/**
 * \name Bits from register OPIOS
 * @{
 */
#define _OPP0IO (1u << 0)
#define _OPP1IO (1u << 1)
#define _OPN0IO (1u << 2)
#define _OPN1IO (1u << 3)
#define _OPN2IO (1u << 4)
/**@}*/

/**
 * \name Bits from register ADCON1
 * @{
 */
#define _GO_DONE (1u << 0)
#define _TRS0    (1u << 1)
#define _TRS1    (1u << 2)
#define _TRS2    (1u << 3)
#define _TRE     (1u << 4)
#define _EC      (1u << 5)
#define _ADCIF   (1u << 6)
#define _ADON    (1u << 7)
/**@}*/

/**
 * \name Bits from register ADCON2
 * @{
 */
#define _ACGIF (1u << 1)
#define _ACLIF (1u << 2)
#define _ACGIE (1u << 3)
#define _ACLIE (1u << 4)
#define _ADCIE (1u << 7)
/**@}*/

/**
 * \name Bits from register SCHCON1
 * @{
 */
#define _SCH0  (1u << 0)
#define _SCH1  (1u << 1)
#define _SCH2  (1u << 2)
#define _SCH3  (1u << 3)
#define _ALR   (1u << 5)
#define _VREF0 (1u << 6)
#define _VREF1 (1u << 7)
/**@}*/

/**
 * \name Bits from register DACCON0
 * @{
 */
#define _DACIF    (1u << 0)
#define _DACIO    (1u << 1)
#define _DACDF    (1u << 2)
#define _DACLSEL0 (1u << 3)
#define _DACLSEL1 (1u << 4)
#define _DACOSEL0 (1u << 5)
#define _DACOSEL1 (1u << 6)
#define _DACEN    (1u << 7)
/**@}*/

/**
 * \name Bits from register DACCON1
 * @{
 */
#define _DACSREF0 (1u << 0)
#define _DACSREF1 (1u << 1)
#define _REFON    (1u << 2)
#define _REFSEL   (1u << 3)
#define _VREFS    (1u << 4)
#define _OFFSETSW (1u << 6)
#define _DACCALON (1u << 7)
/**@}*/

/**
 * \name Bits from register DACCAL
 * @{
 */
#define _OFFSETSIGN (1u << 7)
/**@}*/

/**
 * \name Bits from register P6OS
 * @{
 */
#define _P6OS6 (1u << 6)
#define _P6OS7 (1u << 7)
/**@}*/

/**
 * \name Bits from register P1SS
 * @{
 */
#define _P1S0 (1u << 0)
#define _P1S1 (1u << 1)
#define _P1S2 (1u << 2)
#define _P1S3 (1u << 3)
#define _P1S4 (1u << 4)
#define _P1S5 (1u << 5)
#define _P1S6 (1u << 6)
#define _P1S7 (1u << 7)
/**@}*/

/**
 * \name Bits from register P2SS
 * @{
 */
#define _P2S0 (1u << 0)
#define _P2S1 (1u << 1)
#define _P2S2 (1u << 2)
#define _P2S3 (1u << 3)
#define _P2S4 (1u << 4)
#define _P2S5 (1u << 5)
#define _P2S6 (1u << 6)
#define _P2S7 (1u << 7)
/**@}*/

/**
 * \name Bits from register P3SS
 * @{
 */
#define _P3S0 (1u << 0)
#define _P3S1 (1u << 1)
#define _P3S2 (1u << 2)
#define _P3S3 (1u << 3)
#define _P3S4 (1u << 4)
#define _P3S5 (1u << 5)
#define _P3S6 (1u << 6)
#define _P3S7 (1u << 7)
/**@}*/

/**
 * \name Bits from register P4SS
 * @{
 */
#define _P4S0 (1u << 0)
#define _P4S1 (1u << 1)
#define _P4S2 (1u << 2)
#define _P4S3 (1u << 3)
#define _P4S4 (1u << 4)
#define _P4S5 (1u << 5)
#define _P4S6 (1u << 6)
#define _P4S7 (1u << 7)
/**@}*/

/**
 * \name Bits from register P5SS
 * @{
 */
#define _P5S0 (1u << 0)
#define _P5S1 (1u << 1)
#define _P5S2 (1u << 2)
#define _P5S3 (1u << 3)
#define _P5S4 (1u << 4)
#define _P5S5 (1u << 5)
#define _P5S6 (1u << 6)
#define _P5S7 (1u << 7)
/**@}*/

/**
 * \name Bits from register P6SS
 * @{
 */
#define _P6S0 (1u << 0)
#define _P6S1 (1u << 1)
#define _P6S2 (1u << 2)
#define _P6S3 (1u << 3)
#define _P6S4 (1u << 4)
#define _P6S5 (1u << 5)
#define _P6S6 (1u << 6)
#define _P6S7 (1u << 7)
/**@}*/

/**
 * \name Bits from register P7SS
 * @{
 */
#define _P7S0 (1u << 0)
#define _P7S1 (1u << 1)
/**@}*/

// no pins: an empty direction or pull-up mask.
#define _P_NONE 0x00u

/**
 * \name Bits from registers P0, P0CR, P0PCR
 * @{
 */
#define _P0_0   (1u << 0)
#define _P0_1   (1u << 1)
#define _P0_2   (1u << 2)
#define _P0_3   (1u << 3)
#define _P0_4   (1u << 4)
#define _P0_5   (1u << 5)
#define _P0_6   (1u << 6)
#define _P0_7   (1u << 7)
#define _P0_ALL (uint8_t)(_P0_0 | _P0_1 | _P0_2 | _P0_3 | _P0_4 | _P0_5 | _P0_6 | _P0_7)
/**@}*/

/**
 * \name Bits from registers P1, P1CR, P1PCR
 * @{
 */
#define _P1_0   (1u << 0)
#define _P1_1   (1u << 1)
#define _P1_2   (1u << 2)
#define _P1_3   (1u << 3)
#define _P1_4   (1u << 4)
#define _P1_5   (1u << 5)
#define _P1_6   (1u << 6)
#define _P1_7   (1u << 7)
#define _P1_ALL (uint8_t)(_P1_0 | _P1_1 | _P1_2 | _P1_3 | _P1_4 | _P1_5 | _P1_6 | _P1_7)
/**@}*/

/**
 * \name Bits from registers P2, P2CR, P2PCR
 * @{
 */
#define _P2_0   (1u << 0)
#define _P2_1   (1u << 1)
#define _P2_2   (1u << 2)
#define _P2_3   (1u << 3)
#define _P2_4   (1u << 4)
#define _P2_5   (1u << 5)
#define _P2_6   (1u << 6)
#define _P2_7   (1u << 7)
#define _P2_ALL (uint8_t)(_P2_0 | _P2_1 | _P2_2 | _P2_3 | _P2_4 | _P2_5 | _P2_6 | _P2_7)
/**@}*/

/**
 * \name Bits from registers P3, P3CR, P3PCR
 * @{
 */
#define _P3_0   (1u << 0)
#define _P3_1   (1u << 1)
#define _P3_2   (1u << 2)
#define _P3_3   (1u << 3)
#define _P3_4   (1u << 4)
#define _P3_5   (1u << 5)
#define _P3_6   (1u << 6)
#define _P3_7   (1u << 7)
#define _P3_ALL (uint8_t)(_P3_0 | _P3_1 | _P3_2 | _P3_3 | _P3_4 | _P3_5 | _P3_6 | _P3_7)
/**@}*/

/**
 * \name Bits from registers P4, P4CR, P4PCR
 * @{
 */
#define _P4_0   (1u << 0)
#define _P4_1   (1u << 1)
#define _P4_2   (1u << 2)
#define _P4_3   (1u << 3)
#define _P4_4   (1u << 4)
#define _P4_5   (1u << 5)
#define _P4_6   (1u << 6)
#define _P4_7   (1u << 7)
#define _P4_ALL (uint8_t)(_P4_0 | _P4_1 | _P4_2 | _P4_3 | _P4_4 | _P4_5 | _P4_6 | _P4_7)
/**@}*/

/**
 * \name Bits from registers P5, P5CR, P5PCR
 * @{
 */
#define _P5_0   (1u << 0)
#define _P5_1   (1u << 1)
#define _P5_2   (1u << 2)
#define _P5_3   (1u << 3)
#define _P5_4   (1u << 4)
#define _P5_5   (1u << 5)
#define _P5_6   (1u << 6)
#define _P5_7   (1u << 7)
#define _P5_ALL (uint8_t)(_P5_0 | _P5_1 | _P5_2 | _P5_3 | _P5_4 | _P5_5 | _P5_6 | _P5_7)
/**@}*/

/**
 * \name Bits from registers P6, P6CR, P6PCR
 * @{
 */
#define _P6_0   (1u << 0)
#define _P6_1   (1u << 1)
#define _P6_2   (1u << 2)
#define _P6_3   (1u << 3)
#define _P6_4   (1u << 4)
#define _P6_5   (1u << 5)
#define _P6_6   (1u << 6)
#define _P6_7   (1u << 7)
#define _P6_ALL (uint8_t)(_P6_0 | _P6_1 | _P6_2 | _P6_3 | _P6_4 | _P6_5 | _P6_6 | _P6_7)
/**@}*/

/**
 * \name Bits from registers P7, P7CR, P7PCR
 * @{
 */
#define _P7_0   (1u << 0)
#define _P7_1   (1u << 1)
#define _P7_2   (1u << 2)
#define _P7_3   (1u << 3)
#define _P7_4   (1u << 4)
#define _P7_ALL (uint8_t)(_P7_0 | _P7_1 | _P7_2 | _P7_3 | _P7_4)
/**@}*/

#define USBCON_ENABLE (uint8_t)(_ENUSB | _SW1CON)
#define USBIE1_INIT   (uint8_t)(_OVERIE | _SETUPIE | _RESMIE | _SUSPIE | _PBRSTIE)
#define USBIE2_INIT   (uint8_t)(_OEP0IE | _IEP0IE)

#define EP0_BUF_SIZE 8u
#define EP1_BUF_SIZE 16u
#define EP2_BUF_SIZE 64u

_SBUF(0x0b28) EP0_OUT_BUF[EP0_BUF_SIZE];
_SBUF(0x0b30) EP0_IN_BUF[EP0_BUF_SIZE];

_SBUF(0x0b38) EP1_OUT_BUF[EP1_BUF_SIZE];
_SBUF(0x0b48) EP1_IN_BUF[EP1_BUF_SIZE];

_SBUF(0x0b68) EP2_OUT_BUF[EP2_BUF_SIZE];
_SBUF(0x0ba8) EP2_IN_BUF[EP2_BUF_SIZE];

enum interrupt_index {
    _INT_PCA0     = 0,
    _INT_PCA1     = 1,
    _INT_PCA2     = 2,
    _INT_PCA3     = 3,
    _INT_EUART0   = 4,
    _INT_TIMER2   = 5,
    _INT_ADC      = 6,
    _INT_SPI      = 7,
    _INT_INT2_DAC = 8,
    _INT_INT3     = 9,
    _INT_INT4     = 10,
    _INT_TIMER3   = 11,
    _INT_USB_TWI  = 12,
    _INT_EUART1   = 13,
    _INT_SCM_LPD  = 14,
};
