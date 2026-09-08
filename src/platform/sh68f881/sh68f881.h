#pragma once

#include <stdint.h>
#include <compiler.h>

#define _SBUF(addr) static __xdata __at(addr) volatile uint8_t

// bit 7 is cleared alongside BKS0: the datasheet calls it unimplemented, but dropping it from the mask killed the page-0 ports.
#define INSCON_PAGE_MASK (uint8_t)~(_BKS0 | 0x80u)

#define sfr_page_1() (INSCON |= _BKS0)
#define sfr_page_0() (INSCON &= INSCON_PAGE_MASK)

// CPU
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

// POWER
SFR(PCON, 0x87);
SFR(SUSLO, 0x8e);

// FLASH
SFR(IB_CON1, 0xf2);
SFR(IB_CON2, 0xf3);
SFR(IB_CON3, 0xf4);
SFR(IB_CON4, 0xf5);
SFR(IB_CON5, 0xf6);
SFR(IB_OFFSET, 0xfb);
SFR(IB_DATA, 0xfc);
SFR(XPAGE, 0xf7);
SFR(FLASHCON, 0xa7);

// WDT
SFR(RSTSTAT, 0xb1);

// SYSTEM CLOCK
SFR(CLKCON, 0xb2);
SFR(PLLCON, 0xb3);
SFR(CLKLO, 0xbd);
SFR(CLKRC0, 0xbe);
SFR(CLKRC1, 0xbf);

// INTERRUPTS
SFR(IEN0, 0xa8);
SFR(IEN1, 0xa9);
SFR(IPL0, 0xb8);
SFR(IPL1, 0xb9);
SFR(IPH0, 0xb4);
SFR(IPH1, 0xb5);
SFR(EXF0, 0xe8);
SFR(EXF1, 0xd8);
SFR(IENC, 0xba);
SFR(TCON, 0x88);

// EUART
SFR(SCON, 0x98);
SFR(SBUF, 0x99);
SFR(SADDR, 0x9a);
SFR(SADEN, 0x9b);
SFR(SBRTH, 0x9c);
SFR(SBRTL, 0x9d);
SFR(SFINE, 0xb6);

// TIMER 0/1, page 0
SFR(TMOD, 0x89);
SFR(TL0, 0x8a);
SFR(TL1, 0x8b);
SFR(TH0, 0x8c);
SFR(TH1, 0x8d);
SFR(TCLK_S, 0xce);

// TIMER 2, page 0
SFR(T2CON, 0xc8);
SFR(T2MOD, 0xc9);
SFR(RCAP2L, 0xca);
SFR(RCAP2H, 0xcb);
SFR(TL2, 0xcc);
SFR(TH2, 0xcd);

// TIMER 3/4, page 1
SFR(T3CON, 0x88);
SFR(TL3, 0x8c);
SFR(TH3, 0x8d);
SFR(T4CON, 0xc8);
SFR(TL4, 0xcc);
SFR(TH4, 0xcd);

// BASE TIMER
SFR(BTCON, 0xc1);
SFR(SEC, 0xc2);
SFR(MIN, 0xc3);

// SPI, page 0
SFR(SPCON, 0xa4);
SFR(SPSTA, 0xa5);
SFR(SPDAT, 0xa6);

// REGULATOR
SFR(REGCON, 0xa1);

// ADC, OP, PGA and LCD, all page 0; addresses and names from the SH79F6488/6489 map
SFR(ADDL, 0x91);
SFR(ADCDS, 0x92);
SFR(ADCON, 0x93);
SFR(ADT, 0x94);
SFR(ADCH, 0x95);
SFR(ADDM, 0x96);
SFR(ADDH, 0x97);
SFR(OPCON, 0xa2);
SFR(PGAM, 0xa3);
SFR(LCDCON1, 0xaa);
SFR(LCDCON, 0xab);
SFR(P5SS, 0x9e);
SFR(P6SS, 0x9f);
SFR(P7SS, 0xac);
SFR(P8SS, 0xad);
SFR(PXSS, 0xae);
SFR(SLPCON, 0xaf);

// ISP
SFR(ISPLO, 0xa5);
SFR(ISPCON, 0xa6);

// USB, page 1
SFR(USBCON, 0x99);
SFR(USBIF1, 0xe8);
SFR(USBIF2, 0xf8);
SFR(USBIE1, 0x9a);
SFR(USBIE2, 0x9b);
SFR(USBADDR, 0x9c);
SFR(EP0CON, 0x98);
SFR(EP1CON, 0xc0);
SFR(EP2CON, 0xd8);
SFR(IEP0CNT, 0x9d);
SFR(IEP1CNT, 0x9e);
SFR(IEP2CNT, 0x9f);
SFR(OEP0CNT, 0xa5);
SFR(OEP1CNT, 0xa6);
SFR(OEP2CNT, 0xa7);

// PWM
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
SFR(P8, 0xb0);

SFR(P5CR, 0xe1);
SFR(P6CR, 0xe2);
SFR(P7CR, 0xe3);
SFR(P8CR, 0xe4);

SFR(P5PCR, 0xe9);
SFR(P6PCR, 0xea);
SFR(P7PCR, 0xeb);
SFR(P8PCR, 0xec);

// bits

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
SBIT(P7_5, 0xa0, 5);
SBIT(P7_6, 0xa0, 6);
SBIT(P7_7, 0xa0, 7);

// P8
SBIT(P8_0, 0xb0, 0);
SBIT(P8_1, 0xb0, 1);
SBIT(P8_2, 0xb0, 2);
SBIT(P8_3, 0xb0, 3);
SBIT(P8_4, 0xb0, 4);
SBIT(P8_5, 0xb0, 5);
SBIT(P8_6, 0xb0, 6);
SBIT(P8_7, 0xb0, 7);

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
SBIT(ES, 0xa8, 4);
SBIT(ET3, 0xa8, 3);
SBIT(EX1, 0xa8, 2);
SBIT(ET4, 0xa8, 1);
SBIT(EX0, 0xa8, 0);

// IPL0
SBIT(PADCL, 0xb8, 6);
SBIT(PT2L, 0xb8, 5);
SBIT(PS0L, 0xb8, 4);
SBIT(PT3L, 0xb8, 3);
SBIT(PX1L, 0xb8, 2);
SBIT(PT4L, 0xb8, 1);
SBIT(PX0L, 0xb8, 0);

// EXF0
SBIT(IT41, 0xe8, 7);
SBIT(IT40, 0xe8, 6);
SBIT(IT31, 0xe8, 5);
SBIT(IT30, 0xe8, 4);
SBIT(IT21, 0xe8, 3);
SBIT(IT20, 0xe8, 2);
SBIT(IE3, 0xe8, 1);
SBIT(IE2, 0xe8, 0);

// EXF1
SBIT(IF47, 0xd8, 7);
SBIT(IF46, 0xd8, 6);
SBIT(IF45, 0xd8, 5);
SBIT(IF44, 0xd8, 4);
SBIT(IF43, 0xd8, 3);
SBIT(IF42, 0xd8, 2);
SBIT(IF41, 0xd8, 1);
SBIT(IF40, 0xd8, 0);

// TCON
SBIT(TF1, 0x88, 7);
SBIT(TR1, 0x88, 6);
SBIT(TF0, 0x88, 5);
SBIT(TR0, 0x88, 4);
SBIT(IE1, 0x88, 3);
SBIT(IT1, 0x88, 2);
SBIT(IE0, 0x88, 1);
SBIT(IT0, 0x88, 0);

// the vendor header files ET0 under IEN0, but the address it gives is TCON.4.
SBIT(ET0, 0x88, 4);

// T2CON
SBIT(TF2, 0xc8, 7);
SBIT(EXF2, 0xc8, 6);
SBIT(EXEN2, 0xc8, 3);
SBIT(TR2, 0xc8, 2);
SBIT(C_T2, 0xc8, 1);
SBIT(CP_RL2, 0xc8, 0);

// T3CON
SBIT(TF3, 0x88, 7);
SBIT(T3PS1, 0x88, 5);
SBIT(T3PS0, 0x88, 4);
SBIT(TR3, 0x88, 2);
SBIT(T3CLKS1, 0x88, 1);
SBIT(T3CLKS0, 0x88, 0);

// T4CON
SBIT(TF4, 0xc8, 7);
SBIT(TC4, 0xc8, 6);
SBIT(T4PS1, 0xc8, 5);
SBIT(T4PS0, 0xc8, 4);
SBIT(T4M1, 0xc8, 3);
SBIT(T4M0, 0xc8, 2);
SBIT(TR4, 0xc8, 1);
SBIT(T4CLKS, 0xc8, 0);

// SCON
SBIT(SM0_FE, 0x98, 7);
SBIT(SM1_RXOV, 0x98, 6);
SBIT(SM2_TXCOL, 0x98, 5);
SBIT(REN, 0x98, 4);
SBIT(TB8, 0x98, 3);
SBIT(RB8, 0x98, 2);
SBIT(TI, 0x98, 1);
SBIT(RI, 0x98, 0);

// USBIF1
SBIT(PUPIF, 0xe8, 7);
SBIT(OVERIF, 0xe8, 6);
SBIT(OW, 0xe8, 5);
SBIT(SETUPIF, 0xe8, 4);
SBIT(SOFIF, 0xe8, 3);
SBIT(RESMIF, 0xe8, 2);
SBIT(SUSPIF, 0xe8, 1);
SBIT(USBRSTIF, 0xe8, 0);

// USBIF2
SBIT(OEP2IF, 0xf8, 6);
SBIT(OEP1IF, 0xf8, 5);
SBIT(OEP0IF, 0xf8, 4);
SBIT(IEP2IF, 0xf8, 2);
SBIT(IEP1IF, 0xf8, 1);
SBIT(IEP0IF, 0xf8, 0);

// EP0CON
SBIT(IEP0DTG, 0x98, 7);
SBIT(OEP0DTG, 0x98, 6);
SBIT(IEP0STL, 0x98, 3);
SBIT(IEP0RDY, 0x98, 2);
SBIT(OEP0STL, 0x98, 1);
SBIT(OEP0RDY, 0x98, 0);

// EP1CON
SBIT(IEP1DTG, 0xc0, 7);
SBIT(OEP1DTG, 0xc0, 6);
SBIT(IEP1STL, 0xc0, 3);
SBIT(IEP1RDY, 0xc0, 2);
SBIT(OEP1STL, 0xc0, 1);
SBIT(OEP1RDY, 0xc0, 0);

// EP2CON
SBIT(IEP2DTG, 0xd8, 7);
SBIT(OEP2DTG, 0xd8, 6);
SBIT(IEP2STL, 0xd8, 3);
SBIT(IEP2RDY, 0xd8, 2);
SBIT(OEP2STL, 0xd8, 1);
SBIT(OEP2RDY, 0xd8, 0);

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
#define _PLLFS (1u << 0)
#define _PLLON (1u << 1)
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
 * \name Bits from register IEN0
 * @{
 */
#define _EX0  (1u << 0)
#define _ET4  (1u << 1)
#define _EX1  (1u << 2)
#define _ET3  (1u << 3)
#define _ES   (1u << 4)
#define _ET2  (1u << 5)
#define _EADC (1u << 6)
#define _EA   (1u << 7)
/**@}*/

/**
 * \name Bits from register IEN1
 * @{
 */
#define _ESPI (1u << 0)
#define _EX2  (1u << 1)
#define _EX3  (1u << 2)
#define _EX4  (1u << 3)
#define _EUSB (1u << 4)
#define _EPWM (1u << 5)
#define _EBT  (1u << 6)
#define _ESCM (1u << 7)
/**@}*/

/**
 * \name Bits from register IPH0
 * @{
 */
#define _PX0H  (1u << 0)
#define _PT4H  (1u << 1)
#define _PX1H  (1u << 2)
#define _PT3H  (1u << 3)
#define _PS0H  (1u << 4)
#define _PT2H  (1u << 5)
#define _PADCH (1u << 6)
/**@}*/

/**
 * \name Bits from register IPL0
 * @{
 */
#define _PX0L  (1u << 0)
#define _PT4L  (1u << 1)
#define _PX1L  (1u << 2)
#define _PT3L  (1u << 3)
#define _PS0L  (1u << 4)
#define _PT2L  (1u << 5)
#define _PADCL (1u << 6)
/**@}*/

/**
 * \name Bits from register IPH1
 * @{
 */
#define _PSPIH (1u << 0)
#define _PX2H  (1u << 1)
#define _PX3H  (1u << 2)
#define _PX4H  (1u << 3)
#define _PUSBH (1u << 4)
#define _PPWMH (1u << 5)
#define _PBTH  (1u << 6)
#define _PSCMH (1u << 7)
/**@}*/

/**
 * \name Bits from register IPL1
 * @{
 */
#define _PSPIL (1u << 0)
#define _PX2L  (1u << 1)
#define _PX3L  (1u << 2)
#define _PX4L  (1u << 3)
#define _PUSBL (1u << 4)
#define _PPWML (1u << 5)
#define _PBTL  (1u << 6)
#define _PSCML (1u << 7)
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
 * \name Bits from register TCON
 * @{
 */
#define _IT0 (1u << 0)
#define _IE0 (1u << 1)
#define _IT1 (1u << 2)
#define _IE1 (1u << 3)
#define _TR0 (1u << 4)
#define _TF0 (1u << 5)
#define _TR1 (1u << 6)
#define _TF1 (1u << 7)
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
 * \name Bits from register SBRTH
 * @{
 */
#define _SBRTEN (1u << 7)
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
 * \name Bits from register T4CON
 * @{
 */
#define _T4CLKS (1u << 0)
#define _TR4    (1u << 1)
#define _T4M0   (1u << 2)
#define _T4M1   (1u << 3)
#define _T4PS0  (1u << 4)
#define _T4PS1  (1u << 5)
#define _TC4    (1u << 6)
#define _TF4    (1u << 7)
/**@}*/

/**
 * \name Bits from register BTCON
 * @{
 */
#define _BTS0 (1u << 4)
#define _BTS1 (1u << 5)
#define _BTIF (1u << 6)
#define _BTEN (1u << 7)
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
 * \name Bits from register REGCON
 * @{
 */
#define _REGEN (1u << 0)
#define _REGS  (1u << 1)
/**@}*/

/**
 * \name Bits from register FLASHCON
 * @{
 */
#define _FAC (1u << 0)
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
#define _P7_5   (1u << 5)
#define _P7_6   (1u << 6)
#define _P7_7   (1u << 7)
#define _P7_ALL (uint8_t)(_P7_0 | _P7_1 | _P7_2 | _P7_3 | _P7_4 | _P7_5 | _P7_6 | _P7_7)
/**@}*/

/**
 * \name Bits from registers P8, P8CR, P8PCR
 * @{
 */
#define _P8_0   (1u << 0)
#define _P8_1   (1u << 1)
#define _P8_2   (1u << 2)
#define _P8_3   (1u << 3)
#define _P8_4   (1u << 4)
#define _P8_5   (1u << 5)
#define _P8_6   (1u << 6)
#define _P8_7   (1u << 7)
#define _P8_ALL (uint8_t)(_P8_0 | _P8_1 | _P8_2 | _P8_3 | _P8_4 | _P8_5 | _P8_6 | _P8_7)
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

#define USBCON_ENABLE (uint8_t)(_ENUSB | _SW1CON)
#define USBIE1_INIT   (uint8_t)(_OVERIE | _SETUPIE | _RESMIE | _SUSPIE | _PBRSTIE)
#define USBIE2_INIT   (uint8_t)(_OEP0IE | _IEP0IE)

// USB buffers

#define EP0_BUF_SIZE 8u
#define EP1_BUF_SIZE 16u
#define EP2_BUF_SIZE 64u

_SBUF(0x0a00) EP0_OUT_BUF[EP0_BUF_SIZE];
_SBUF(0x0a08) EP0_IN_BUF[EP0_BUF_SIZE];

_SBUF(0x0a10) EP1_OUT_BUF[EP1_BUF_SIZE];
_SBUF(0x0a20) EP1_IN_BUF[EP1_BUF_SIZE];

_SBUF(0x0a30) EP2_OUT_BUF[EP2_BUF_SIZE];
_SBUF(0x0a70) EP2_IN_BUF[EP2_BUF_SIZE];

enum interrupt_index {
    _INT_INT0       = 0,
    _INT_TIMER4     = 1,
    _INT_INT1       = 2,
    _INT_TIMER3     = 3,
    _INT_EUART      = 4,
    _INT_TIMER2     = 5,
    _INT_ADC        = 6,
    _INT_SPI        = 7,
    _INT_INT2       = 8,
    _INT_INT3       = 9,
    _INT_INT4       = 10,
    _INT_USB        = 11,
    _INT_PWM        = 12,
    _INT_BASE_TIMER = 13,
    _INT_SCM        = 14,
};
