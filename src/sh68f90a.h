#ifndef SH68F90A_H
#define SH68F90A_H

#include <compiler.h>
#include <stdint.h>

#define _SBUF(addr) static __xdata __at(addr) volatile uint8_t

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
SFR(IB_OFFSET, 0xfb);
SFR(IB_DATA, 0xfc);
SFR(IB_CON1, 0xf2);
SFR(IB_CON2, 0xf3);
SFR(IB_CON3, 0xf4);
SFR(IB_CON4, 0xf5);
SFR(IB_CON5, 0xf6);
SFR(XPAGE, 0xf7);
SFR(FLASHCON, 0xa7);

// WDT
SFR(RSTSTAT, 0xb1);

// SYSTEM CLOCK
SFR(CLKCON, 0xb2);
SFR(PLLCON, 0xbc);
SFR(CLKLO, 0xbd);
SFR(CLKRC0L, 0xbe);
SFR(CLKRC0H, 0xc6);
SFR(CLKRC1L, 0xbf);
SFR(CLKRC1H, 0xc7);

// INTERRUPTS
SFR(IEN0, 0xa8);
SFR(IEN1, 0xa9);
SFR(IPL0, 0xb8);
SFR(IPL1, 0xb9);
SFR(IPH0, 0xb4);
SFR(IPH1, 0xb5);
SFR(EXF0, 0xb6);
SFR(EXF1, 0xe8);
SFR(IENC, 0xba);
SFR(EXCON, 0x8b);

// EUART
SFR(SCON, 0xd8);
SFR(SBUF, 0xaa);
SFR(SADDR, 0xab);
SFR(SADEN, 0xac);
SFR(SBRTH, 0xad);
SFR(SBRTL, 0xae);
SFR(SFINE, 0xaf);

// TIMER2
SFR(T2CON, 0xc8);
SFR(T2MOD, 0xc9);
SFR(RCAP2L, 0xca);
SFR(RCAP2H, 0xcb);
SFR(TL2, 0xcc);
SFR(TH2, 0xcd);

// SPI
SFR(SPCON, 0xa2);
SFR(SPSTA, 0xa1);
SFR(SPDAT, 0xa3);

// REGULATOR
SFR(REGCON, 0x8f);

// USB
SFR(USBCON, 0x91);
SFR(USBIF1, 0x92);
SFR(USBIF2, 0x93);
SFR(USBIE1, 0x94);
SFR(USBIE2, 0x95);
SFR(USBADDR, 0x96);
SFR(EP0CON, 0x97);
SFR(EP1CON, 0x99);
SFR(EP2CON, 0x9a);
SFR(IEP0CNT, 0x9b);
SFR(IEP1CNT, 0x9c);
SFR(IEP2CNT, 0x9d);
SFR(OEP0CNT, 0x9e);
SFR(OEP1CNT, 0x9f);
SFR(OEP2CNT, 0xa4);

// LPD
SFR(LPDCON, 0xb3);
SFR(LPDSEL, 0x89);

// DRIVE OPTION
SFR(DRVCON, 0x8c);
SFR(P1DRV, 0xa5);
SFR(P2DRV, 0xa6);
SFR(P3DRV, 0xbb);
SFR(P5DRV, 0x8d);

// MAPPING
SFR(MAPPING, 0x8a);

// PWM0
SFRX(PWM00CON, 0xff80);
SFRX(PWM01CON, 0xff81);
SFRX(PWM02CON, 0xff82);
SFRX(PWM03CON, 0xff83);
SFRX(PWM04CON, 0xff84);
SFRX(PWM05CON, 0xff85);

SFRX(PWM0PERDL, 0xff98);
SFRX(PWM0PERDH, 0xff9c);

SFRX(PWM00DUTY1L, 0xffa0);
SFRX(PWM00DUTY1H, 0xffb8);
SFRX(PWM00DUTY2L, 0xffd0);
SFRX(PWM00DUTY2H, 0xffe8);

SFRX(PWM01DUTY1L, 0xffa1);
SFRX(PWM01DUTY1H, 0xffb9);
SFRX(PWM01DUTY2L, 0xffd1);
SFRX(PWM01DUTY2H, 0xffe9);

SFRX(PWM02DUTY1L, 0xffa2);
SFRX(PWM02DUTY1H, 0xffba);
SFRX(PWM02DUTY2L, 0xffd2);
SFRX(PWM02DUTY2H, 0xffea);

SFRX(PWM03DUTY1L, 0xffa3);
SFRX(PWM03DUTY1H, 0xffbb);
SFRX(PWM03DUTY2L, 0xffd3);
SFRX(PWM03DUTY2H, 0xffeb);

SFRX(PWM04DUTY1L, 0xffa4);
SFRX(PWM04DUTY1H, 0xffbc);
SFRX(PWM04DUTY2L, 0xffd4);
SFRX(PWM04DUTY2H, 0xffec);

SFRX(PWM05DUTY1L, 0xffa5);
SFRX(PWM05DUTY1H, 0xffbd);
SFRX(PWM05DUTY2L, 0xffd5);
SFRX(PWM05DUTY2H, 0xffed);

// PWM1
SFRX(PWM10CON, 0xff86);
SFRX(PWM11CON, 0xff87);
SFRX(PWM12CON, 0xff88);
SFRX(PWM13CON, 0xff89);
SFRX(PWM14CON, 0xff8a);
SFRX(PWM15CON, 0xff8b);

SFRX(PWM1PERDL, 0xff99);
SFRX(PWM1PERDH, 0xff9d);

SFRX(PWM10DUTY1L, 0xffa6);
SFRX(PWM10DUTY1H, 0xffbe);
SFRX(PWM10DUTY2L, 0xffd6);
SFRX(PWM10DUTY2H, 0xffee);

SFRX(PWM11DUTY1L, 0xffa7);
SFRX(PWM11DUTY1H, 0xffbf);
SFRX(PWM11DUTY2L, 0xffd7);
SFRX(PWM11DUTY2H, 0xffef);

SFRX(PWM12DUTY1L, 0xffa8);
SFRX(PWM12DUTY1H, 0xffc0);
SFRX(PWM12DUTY2L, 0xffd8);
SFRX(PWM12DUTY2H, 0xfff0);

SFRX(PWM13DUTY1L, 0xffa9);
SFRX(PWM13DUTY1H, 0xffc1);
SFRX(PWM13DUTY2L, 0xffd9);
SFRX(PWM13DUTY2H, 0xfff1);

SFRX(PWM14DUTY1L, 0xffaa);
SFRX(PWM14DUTY1H, 0xffc2);
SFRX(PWM14DUTY2L, 0xffda);
SFRX(PWM14DUTY2H, 0xfff2);

SFRX(PWM15DUTY1L, 0xffab);
SFRX(PWM15DUTY1H, 0xffc3);
SFRX(PWM15DUTY2L, 0xffdb);
SFRX(PWM15DUTY2H, 0xfff3);

// PWM2
SFRX(PWM20CON, 0xff8c);
SFRX(PWM21CON, 0xff8d);
SFRX(PWM22CON, 0xff8e);
SFRX(PWM23CON, 0xff8f);
SFRX(PWM24CON, 0xff90);
SFRX(PWM25CON, 0xff91);

SFRX(PWM2PERDL, 0xff9a);
SFRX(PWM2PERDH, 0xff9e);

SFRX(PWM20DUTY1L, 0xffac);
SFRX(PWM20DUTY1H, 0xffc4);
SFRX(PWM20DUTY2L, 0xffdc);
SFRX(PWM20DUTY2H, 0xfff4);

SFRX(PWM21DUTY1L, 0xffad);
SFRX(PWM21DUTY1H, 0xffc5);
SFRX(PWM21DUTY2L, 0xffdd);
SFRX(PWM21DUTY2H, 0xfff5);

SFRX(PWM22DUTY1L, 0xffae);
SFRX(PWM22DUTY1H, 0xffc6);
SFRX(PWM22DUTY2L, 0xffde);
SFRX(PWM22DUTY2H, 0xfff6);

SFRX(PWM23DUTY1L, 0xffaf);
SFRX(PWM23DUTY1H, 0xffc7);
SFRX(PWM23DUTY2L, 0xffdf);
SFRX(PWM23DUTY2H, 0xfff7);

SFRX(PWM24DUTY1L, 0xffb0);
SFRX(PWM24DUTY1H, 0xffc8);
SFRX(PWM24DUTY2L, 0xffe0);
SFRX(PWM24DUTY2H, 0xfff8);

SFRX(PWM25DUTY1L, 0xffb1);
SFRX(PWM25DUTY1H, 0xffc9);
SFRX(PWM25DUTY2L, 0xffe1);
SFRX(PWM25DUTY2H, 0xfff9);

// PWM3
SFRX(PWM30CON, 0xff92);
SFRX(PWM31CON, 0xff93);
SFRX(PWM32CON, 0xff94);
SFRX(PWM33CON, 0xff95);

SFRX(PWM3PERDL, 0xff9b);
SFRX(PWM3PERDH, 0xff9f);

SFRX(PWM30DUTY1L, 0xffb2);
SFRX(PWM30DUTY1H, 0xffca);
SFRX(PWM30DUTY2L, 0xffe2);
SFRX(PWM30DUTY2H, 0xfffa);

SFRX(PWM31DUTY1L, 0xffb3);
SFRX(PWM31DUTY1H, 0xffcb);
SFRX(PWM31DUTY2L, 0xffe3);
SFRX(PWM31DUTY2H, 0xfffb);

SFRX(PWM32DUTY1L, 0xffb4);
SFRX(PWM32DUTY1H, 0xffcc);
SFRX(PWM32DUTY2L, 0xffe4);
SFRX(PWM32DUTY2H, 0xfffc);

SFRX(PWM33DUTY1L, 0xffb5);
SFRX(PWM33DUTY1H, 0xffcd);
SFRX(PWM33DUTY2L, 0xffe5);
SFRX(PWM33DUTY2H, 0xfffd);

// PWM4
SFR(PWM40CON, 0xda);
SFR(PWM41CON, 0xdb);
SFR(PWM42CON, 0xdc);

SFR(PWM4PERDL, 0xdd);
SFR(PWM4PERDH, 0xde);

SFR(PWM40DUTY1L, 0xd2);
SFR(PWM41DUTY1L, 0xd3);
SFR(PWM42DUTY1L, 0xd4);

SFR(PWM40DUTY1H, 0xd5);
SFR(PWM41DUTY1H, 0xd6);
SFR(PWM42DUTY1H, 0xd7);

SFR(PWM40DUTY2L, 0xce);
SFR(PWM41DUTY2L, 0xc1);
SFR(PWM42DUTY2L, 0xc2);

SFR(PWM40DUTY2H, 0xc3);
SFR(PWM41DUTY2H, 0xc4);
SFR(PWM42DUTY2H, 0xc5);

// PORT
SFR(P0, 0x80);
SFR(P1, 0x90);
SFR(P2, 0x98);
SFR(P3, 0xa0);
SFR(P4, 0xb0);
SFR(P5, 0x88);
SFR(P6, 0xc0);
SFR(P7, 0xf8);

SFR(P0CR, 0xe1);
SFR(P1CR, 0xe2);
SFR(P2CR, 0xe3);
SFR(P3CR, 0xe4);
SFR(P4CR, 0xe5);
SFR(P5CR, 0xe6);
SFR(P6CR, 0xe7);
SFR(P7CR, 0xd1);

SFR(P0PCR, 0xe9);
SFR(P1PCR, 0xea);
SFR(P2PCR, 0xeb);
SFR(P3PCR, 0xec);
SFR(P4PCR, 0xed);
SFR(P5PCR, 0xee);
SFR(P6PCR, 0xef);
SFR(P7PCR, 0xd9);

// Bits

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

// P2
SBIT(P2_0, 0x98, 0);
SBIT(P2_1, 0x98, 1);
SBIT(P2_2, 0x98, 2);
SBIT(P2_3, 0x98, 3);
SBIT(P2_4, 0x98, 4);
SBIT(P2_5, 0x98, 5);

// P3
SBIT(P3_0, 0xa0, 0);
SBIT(P3_1, 0xa0, 1);
SBIT(P3_2, 0xa0, 2);
SBIT(P3_3, 0xa0, 3);
SBIT(P3_4, 0xa0, 4);
SBIT(P3_5, 0xa0, 5);

// P4
SBIT(P4_0, 0xb0, 0);
SBIT(P4_1, 0xb0, 1);
SBIT(P4_2, 0xb0, 2);
SBIT(P4_3, 0xb0, 3);
SBIT(P4_4, 0xb0, 4);
SBIT(P4_5, 0xb0, 5);
SBIT(P4_6, 0xb0, 6);
SBIT(P4_7, 0xb0, 7);

// P5
SBIT(P5_0, 0x88, 0);
SBIT(P5_1, 0x88, 1);
SBIT(P5_2, 0x88, 2);
SBIT(P5_3, 0x88, 3);
SBIT(P5_4, 0x88, 4);
SBIT(P5_5, 0x88, 5);
SBIT(P5_6, 0x88, 6);
SBIT(P5_7, 0x88, 7);

// P6
SBIT(P6_0, 0xc0, 0);
SBIT(P6_1, 0xc0, 1);
SBIT(P6_2, 0xc0, 2);
SBIT(P6_3, 0xc0, 3);
SBIT(P6_4, 0xc0, 4);
SBIT(P6_5, 0xc0, 5);
SBIT(P6_6, 0xc0, 6);
SBIT(P6_7, 0xc0, 7);

// P7
SBIT(P7_0, 0xf8, 0);
SBIT(P7_1, 0xf8, 1);
SBIT(P7_2, 0xf8, 2);
SBIT(P7_3, 0xf8, 3);
SBIT(P7_4, 0xf8, 4);
SBIT(P7_5, 0xf8, 5);
SBIT(P7_6, 0xf8, 6);
SBIT(P7_7, 0xf8, 7);

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
SBIT(ESPI, 0xa8, 6);
SBIT(ELPD, 0xa8, 5);
SBIT(ESCM, 0xa8, 4);
SBIT(EX2, 0xa8, 3);
SBIT(EX3, 0xa8, 2);
SBIT(EX4, 0xa8, 1);
SBIT(ET2, 0xa8, 0);

// T2CON
SBIT(TF2, 0xc8, 7);
SBIT(EXF2, 0xc8, 6);
SBIT(EXEN2, 0xc8, 3);
SBIT(TR2, 0xc8, 2);
SBIT(C_T2, 0xc8, 1);
SBIT(CP_RL2, 0xc8, 0);

// SCON
SBIT(SM0_FE, 0xd8, 7);
SBIT(SM1_RXOV, 0xd8, 6);
SBIT(SM2_TXCOL, 0xd8, 5);
SBIT(REN, 0xd8, 4);
SBIT(TB8, 0xd8, 3);
SBIT(RB8, 0xd8, 2);
SBIT(TI, 0xd8, 1);
SBIT(RI, 0xd8, 0);

// EXF1
SBIT(IF47, 0xe8, 7);
SBIT(IF46, 0xe8, 6);
SBIT(IF45, 0xe8, 5);
SBIT(IF44, 0xe8, 4);
SBIT(IF43, 0xe8, 3);
SBIT(IF42, 0xe8, 2);
SBIT(IF41, 0xe8, 1);
SBIT(IF40, 0xe8, 0);

// ACC
SBIT(ACC_7, 0xe0, 7);
SBIT(ACC_6, 0xe0, 6);
SBIT(ACC_5, 0xe0, 5);
SBIT(ACC_4, 0xe0, 4);
SBIT(ACC_3, 0xe0, 3);
SBIT(ACC_2, 0xe0, 2);
SBIT(ACC_1, 0xe0, 1);
SBIT(ACC_0, 0xe0, 0);

// B
SBIT(B_7, 0xf0, 7);
SBIT(B_6, 0xf0, 6);
SBIT(B_5, 0xf0, 5);
SBIT(B_4, 0xf0, 4);
SBIT(B_3, 0xf0, 3);
SBIT(B_2, 0xf0, 2);
SBIT(B_1, 0xf0, 1);
SBIT(B_0, 0xf0, 0);

// IPL0
SBIT(PT2L, 0x88, 0);
SBIT(PX4L, 0x88, 1);
SBIT(PX3L, 0x88, 2);
SBIT(PX2L, 0x88, 3);
SBIT(PSCML, 0x88, 4);
SBIT(PLPDL, 0x88, 5);
SBIT(PSPIL, 0x88, 6);

/**
 * \name Bits from register RSTSTAT
 * @{
 */
#define _CLRF (1u << 3) ///< Bit 3
#define _LVRF (1u << 4) ///< Bit 4
#define _PORF (1u << 5) ///< Bit 5
#define _WDOF (1u << 7) ///< Bit 7
/**@}*/

/**
 * \name Bits from register CLKCON
 * @{
 */
#define _FS   (1u << 2) ///< Bit 2
#define _HFON (1u << 3) ///< Bit 3
/**@}*/

/**
 * \name Bits from register PLLCON
 * @{
 */
#define _PLLFS  (1u << 0) ///< Bit 0
#define _PLLON  (1u << 1) ///< Bit 1
#define _PLLSTA (1u << 2) ///< Bit 2
/**@}*/

/**
 * \name Bits from register IEN0
 * @{
 */
#define _ET2  (1u << 0) ///< Bit 0
#define _EX4  (1u << 1) ///< Bit 1
#define _EX3  (1u << 2) ///< Bit 2
#define _EX2  (1u << 3) ///< Bit 3
#define _ESCM (1u << 4) ///< Bit 4
#define _ELPD (1u << 5) ///< Bit 5
#define _ESPI (1u << 6) ///< Bit 6
#define _EA   (1u << 7) ///< Bit 7
/**@}*/

/**
 * \name Bits from register IEN1
 * @{
 */
#define _EUSB  (1u << 0) ///< Bit 0
#define _EPWM0 (1u << 1) ///< Bit 1
#define _EPWM1 (1u << 2) ///< Bit 2
#define _EPWM2 (1u << 3) ///< Bit 3
#define _EPWM3 (1u << 4) ///< Bit 4
#define _EPWM4 (1u << 5) ///< Bit 5
#define _ES0   (1u << 6) ///< Bit 6
/**@}*/

/**
 * \name Bits from registers P0, P0CR, P0PCR
 * @{
 */
#define _P0_0 (1u << 0) ///< Bit 0
#define _P0_1 (1u << 1) ///< Bit 1
#define _P0_2 (1u << 2) ///< Bit 2
#define _P0_3 (1u << 3) ///< Bit 3
#define _P0_4 (1u << 4) ///< Bit 4
#define _P0_5 (1u << 5) ///< Bit 5
#define _P0_6 (1u << 6) ///< Bit 6
#define _P0_7 (1u << 7) ///< Bit 7
/**@}*/

/**
 * \name Bits from registers P1, P1CR, P1PCR
 * @{
 */
#define _P1_0 (1u << 0) ///< Bit 0
#define _P1_1 (1u << 1) ///< Bit 1
#define _P1_2 (1u << 2) ///< Bit 2
#define _P1_3 (1u << 3) ///< Bit 3
#define _P1_4 (1u << 4) ///< Bit 4
#define _P1_5 (1u << 5) ///< Bit 5
/**@}*/

/**
 * \name Bits from registers P2, P2CR, P2PCR
 * @{
 */
#define _P2_0 (1u << 0) ///< Bit 0
#define _P2_1 (1u << 1) ///< Bit 1
#define _P2_2 (1u << 2) ///< Bit 2
#define _P2_3 (1u << 3) ///< Bit 3
#define _P2_4 (1u << 4) ///< Bit 4
#define _P2_5 (1u << 5) ///< Bit 5
/**@}*/

/**
 * \name Bits from registers P3, P3CR, P3PCR
 * @{
 */
#define _P3_0 (1u << 0) ///< Bit 0
#define _P3_1 (1u << 1) ///< Bit 1
#define _P3_2 (1u << 2) ///< Bit 2
#define _P3_3 (1u << 3) ///< Bit 3
#define _P3_4 (1u << 4) ///< Bit 4
#define _P3_5 (1u << 5) ///< Bit 5
/**@}*/

/**
 * \name Bits from registers P4, P4CR, P4PCR
 * @{
 */
#define _P4_0 (1u << 0) ///< Bit 0
#define _P4_1 (1u << 1) ///< Bit 1
#define _P4_2 (1u << 2) ///< Bit 2
#define _P4_3 (1u << 3) ///< Bit 3
#define _P4_4 (1u << 4) ///< Bit 4
#define _P4_5 (1u << 5) ///< Bit 5
#define _P4_6 (1u << 6) ///< Bit 6
#define _P4_7 (1u << 7) ///< Bit 7
/**@}*/

/**
 * \name Bits from registers P5, P5CR, P5PCR
 * @{
 */
#define _P5_0 (1u << 0) ///< Bit 0
#define _P5_1 (1u << 1) ///< Bit 1
#define _P5_2 (1u << 2) ///< Bit 2
#define _P5_3 (1u << 3) ///< Bit 3
#define _P5_4 (1u << 4) ///< Bit 4
#define _P5_5 (1u << 5) ///< Bit 5
#define _P5_6 (1u << 6) ///< Bit 6
#define _P5_7 (1u << 7) ///< Bit 7
/**@}*/

/**
 * \name Bits from registers P6, P6CR, P6PCR
 * @{
 */
#define _P6_0 (1u << 0) ///< Bit 0
#define _P6_1 (1u << 1) ///< Bit 1
#define _P6_2 (1u << 2) ///< Bit 2
#define _P6_3 (1u << 3) ///< Bit 3
#define _P6_4 (1u << 4) ///< Bit 4
#define _P6_5 (1u << 5) ///< Bit 5
#define _P6_6 (1u << 6) ///< Bit 6
#define _P6_7 (1u << 7) ///< Bit 7
/**@}*/

/**
 * \name Bits from registers P7, P7CR, P7PCR
 * @{
 */
#define _P7_0 (1u << 0) ///< Bit 0
#define _P7_1 (1u << 1) ///< Bit 1
#define _P7_2 (1u << 2) ///< Bit 2
#define _P7_3 (1u << 3) ///< Bit 3
#define _P7_4 (1u << 4) ///< Bit 4
#define _P7_5 (1u << 5) ///< Bit 5
#define _P7_6 (1u << 6) ///< Bit 6
#define _P7_7 (1u << 7) ///< Bit 7
/**@}*/

/**
 * \name Bits from register REGCON
 * @{
 */
#define _REGEN (1u << 0) ///< Bit 0
/**@}*/

/**
 * \name Bits from register USBCON
 * @{
 */
#define _GOSUSP (1u << 0) ///< Bit 0
#define _WKUP   (1u << 1) ///< Bit 1
#define _SW2CON (1u << 2) ///< Bit 2
#define _DMSTA  (1u << 3) ///< Bit 3
#define _DPSTA  (1u << 4) ///< Bit 4
#define _SWRST  (1u << 5) ///< Bit 5
#define _SW1CON (1u << 6) ///< Bit 6
#define _ENUSB  (1u << 7) ///< Bit 7
/**@}*/

/**
 * \name Bits from register USBIF1
 * @{
 */
#define _USBRSTIF (1u << 0) ///< Bit 0
#define _SUSPIF   (1u << 1) ///< Bit 1
#define _RESMIF   (1u << 2) ///< Bit 2
#define _SOFIF    (1u << 3) ///< Bit 3
#define _SETUPIF  (1u << 4) ///< Bit 4
#define _OW       (1u << 5) ///< Bit 5
#define _OVERIF   (1u << 6) ///< Bit 6
#define _PUPIF    (1u << 7) ///< Bit 7
/**@}*/

/**
 * \name Bits from register USBIF2
 * @{
 */
#define _IEP0IF (1u << 0) ///< Bit 0
#define _IEP1IF (1u << 1) ///< Bit 1
#define _IEP2IF (1u << 2) ///< Bit 2
#define _OEP0IF (1u << 4) ///< Bit 4
#define _OEP1IF (1u << 5) ///< Bit 5
#define _OEP2IF (1u << 6) ///< Bit 6
/**@}*/

/**
 * \name Bits from register USBIE1
 * @{
 */
#define _PBRSTIE (1u << 0) ///< Bit 0
#define _SUSPIE  (1u << 1) ///< Bit 1
#define _RESMIE  (1u << 2) ///< Bit 2
#define _SOFIA   (1u << 3) ///< Bit 3
#define _SETUPIE (1u << 4) ///< Bit 4
#define _OVERIE  (1u << 6) ///< Bit 6
#define _BOOTS   (1u << 7) ///< Bit 7
/**@}*/

/**
 * \name Bits from register USBIE2
 * @{
 */
#define _IEP0IE (1u << 0) ///< Bit 0
#define _IEP1IE (1u << 1) ///< Bit 1
#define _IEP2IE (1u << 2) ///< Bit 2
#define _OEP0IE (1u << 4) ///< Bit 4
#define _OEP1IE (1u << 5) ///< Bit 5
#define _OEP2IE (1u << 6) ///< Bit 6
/**@}*/

/**
 * \name Bits from register EP0CON
 * @{
 */
#define _OEP0RDY (1u << 0) ///< Bit 0
#define _OEP0STL (1u << 1) ///< Bit 1
#define _IEP0RDY (1u << 2) ///< Bit 2
#define _IEP0STL (1u << 3) ///< Bit 3
#define _OEP0DTG (1u << 6) ///< Bit 6
#define _IEP0DTG (1u << 7) ///< Bit 7
/**@}*/

/**
 * \name Bits from register EP1CON
 * @{
 */
#define _OEP1RDY    (1u << 0) ///< Bit 0
#define _OEP1STL    (1u << 1) ///< Bit 1
#define _IEP1RDY    (1u << 2) ///< Bit 2
#define _IEP1STL    (1u << 3) ///< Bit 3
#define _OEP1BUFSEL (1u << 4) ///< Bit 4
#define _IEP1BUFSEL (1u << 5) ///< Bit 5
#define _OEP1DTG    (1u << 6) ///< Bit 6
#define _IEP1DTG    (1u << 7) ///< Bit 7
/**@}*/

/**
 * \name Bits from register EP2CON
 * @{
 */
#define _OEP2RDY    (1u << 0) ///< Bit 0
#define _OEP2STL    (1u << 1) ///< Bit 1
#define _IEP2RDY    (1u << 2) ///< Bit 2
#define _IEP2STL    (1u << 3) ///< Bit 3
#define _OEP2BUFSEL (1u << 4) ///< Bit 4
#define _IEP2BUFSEL (1u << 5) ///< Bit 5
#define _OEP2DTG    (1u << 6) ///< Bit 6
#define _IEP2DTG    (1u << 7) ///< Bit 7
/**@}*/

/**
 * \name Bits from register SCON
 * @{
 */
#define _RI  (1u << 0) ///< Bit 0
#define _TI  (1u << 1) ///< Bit 1
#define _RB8 (1u << 2) ///< Bit 2
#define _TB8 (1u << 3) ///< Bit 3
#define _REN (1u << 4) ///< Bit 4
#define _SM2 (1u << 5) ///< Bit 5
#define _SM1 (1u << 6) ///< Bit 6
#define _SM0 (1u << 7) ///< Bit 7
/**@}*/

/**
 * \name Bits from register PCON
 * @{
 */
#define _SSTAT (1u << 6) ///< Bit 6
#define _SMOD  (1u << 7) ///< Bit 7
/**@}*/

/**
 * \name Bits from register SBRTH
 * @{
 */
#define _SBRTEN (1u << 7) ///< Bit 7
/**@}*/

// USB Buffers

#define EP0_BUF_SIZE 8
#define EP1_BUF_SIZE 16
#define EP2_BUF_SIZE 64

_SBUF(0x1100) EP0_OUT_BUF[EP0_BUF_SIZE];
_SBUF(0x1108) EP0_IN_BUF[EP0_BUF_SIZE];

_SBUF(0x1110) EP1_OUT_BUF[EP1_BUF_SIZE];
_SBUF(0x1120) EP1_IN_BUF[EP1_BUF_SIZE];
_SBUF(0x1130) EP1_COM_BUF[EP1_BUF_SIZE];

_SBUF(0x1140) EP2_OUT_BUF[EP2_BUF_SIZE];
_SBUF(0x1180) EP2_IN_BUF[EP2_BUF_SIZE];
_SBUF(0x11c0) EP2_COM_BUF[EP2_BUF_SIZE];

enum interrupt_index {
    _INT_TIMER2 = 0,
    _INT_INT4   = 1,
    _INT_INT3   = 2,
    _INT_INT2   = 3,
    _INT_LPD    = 5,
    _INT_SPI    = 6,
    _INT_USB    = 7,
    _INT_PWM0   = 8,
    _INT_PWM1   = 9,
    _INT_PWM2   = 10,
    _INT_PWM3   = 11,
    _INT_PWM4   = 12,
    _INT_EUART0 = 13,
};

#endif
