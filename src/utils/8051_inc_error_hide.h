#pragma once

/*
	To solve the problem that vs / clang dose not
	recognize those SDCC defined keywords.
*/

#ifdef __SDCC
	// #include <mcs51/8051.h>

#else
	//keywords
	#define __interrupt
	#define __using
	#define __code
	#define __data
	#define __near
	#define __xdata
	#define __far
	#define __idata
	#define __pdata
	#define __code
	#define __bit
	#define __sfr
	#define __sfr16
	#define __sfr32
	#define __sbit
	#define __at
	#define __critical
	//#define __asm
	//#define __endasm

	//header - 8051.h registers

	#define P0 (*(char*)((0x80)))
	#define SP (*(char*)((0x81)))
	#define DPL (*(char*)((0x82)))
	#define DPH (*(char*)((0x83)))
	#define PCON (*(char*)((0x87)))
	#define TCON (*(char*)((0x88)))
	#define TMOD (*(char*)((0x89)))
	#define TL0 (*(char*)((0x8A)))
	#define TL1 (*(char*)((0x8B)))
	#define TH0 (*(char*)((0x8C)))
	#define TH1 (*(char*)((0x8D)))
	#define P1 (*(char*)((0x90)))
	#define SCON (*(char*)((0x98)))
	#define SBUF (*(char*)((0x99)))
	#define P2 (*(char*)((0xA0)))
	#define IE (*(char*)((0xA8)))
	#define P3 (*(char*)((0xB0)))
	#define IP (*(char*)((0xB8)))
	#define PSW (*(char*)((0xD0)))
	#define ACC (*(char*)((0xE0)))
	#define B (*(char*)((0xF0)))
	#define P0_0 (*(char*)((0x80)))
	#define P0_1 (*(char*)((0x81)))
	#define P0_2 (*(char*)((0x82)))
	#define P0_3 (*(char*)((0x83)))
	#define P0_4 (*(char*)((0x84)))
	#define P0_5 (*(char*)((0x85)))
	#define P0_6 (*(char*)((0x86)))
	#define P0_7 (*(char*)((0x87)))
	#define IT0 (*(char*)((0x88)))
	#define IE0 (*(char*)((0x89)))
	#define IT1 (*(char*)((0x8A)))
	#define IE1 (*(char*)((0x8B)))
	#define TR0 (*(char*)((0x8C)))
	#define TF0 (*(char*)((0x8D)))
	#define TR1 (*(char*)((0x8E)))
	#define TF1 (*(char*)((0x8F)))
	#define P1_0 (*(char*)((0x90)))
	#define P1_1 (*(char*)((0x91)))
	#define P1_2 (*(char*)((0x92)))
	#define P1_3 (*(char*)((0x93)))
	#define P1_4 (*(char*)((0x94)))
	#define P1_5 (*(char*)((0x95)))
	#define P1_6 (*(char*)((0x96)))
	#define P1_7 (*(char*)((0x97)))
	#define RI (*(char*)((0x98)))
	#define TI (*(char*)((0x99)))
	#define RB8 (*(char*)((0x9A)))
	#define TB8 (*(char*)((0x9B)))
	#define REN (*(char*)((0x9C)))
	#define SM2 (*(char*)((0x9D)))
	#define SM1 (*(char*)((0x9E)))
	#define SM0 (*(char*)((0x9F)))
	#define P2_0 (*(char*)((0xA0)))
	#define P2_1 (*(char*)((0xA1)))
	#define P2_2 (*(char*)((0xA2)))
	#define P2_3 (*(char*)((0xA3)))
	#define P2_4 (*(char*)((0xA4)))
	#define P2_5 (*(char*)((0xA5)))
	#define P2_6 (*(char*)((0xA6)))
	#define P2_7 (*(char*)((0xA7)))
	#define EX0 (*(char*)((0xA8)))
	#define ET0 (*(char*)((0xA9)))
	#define EX1 (*(char*)((0xAA)))
	#define ET1 (*(char*)((0xAB)))
	#define ES (*(char*)((0xAC)))
	#define EA (*(char*)((0xAF)))
	#define P3_0 (*(char*)((0xB0)))
	#define P3_1 (*(char*)((0xB1)))
	#define P3_2 (*(char*)((0xB2)))
	#define P3_3 (*(char*)((0xB3)))
	#define P3_4 (*(char*)((0xB4)))
	#define P3_5 (*(char*)((0xB5)))
	#define P3_6 (*(char*)((0xB6)))
	#define P3_7 (*(char*)((0xB7)))
	#define RXD (*(char*)((0xB0)))
	#define TXD (*(char*)((0xB1)))
	#define INT0 (*(char*)((0xB2)))
	#define INT1 (*(char*)((0xB3)))
	#define T0 (*(char*)((0xB4)))
	#define T1 (*(char*)((0xB5)))
	#define WR (*(char*)((0xB6)))
	#define RD (*(char*)((0xB7)))
	#define PX0 (*(char*)((0xB8)))
	#define PT0 (*(char*)((0xB9)))
	#define PX1 (*(char*)((0xBA)))
	#define PT1 (*(char*)((0xBB)))
	#define PS (*(char*)((0xBC)))
	#define P (*(char*)((0xD0)))
	#define F1 (*(char*)((0xD1)))
	#define OV (*(char*)((0xD2)))
	#define RS0 (*(char*)((0xD3)))
	#define RS1 (*(char*)((0xD4)))
	#define F0 (*(char*)((0xD5)))
	#define AC (*(char*)((0xD6)))
	#define CY (*(char*)((0xD7)))

	/* BIT definitions for bits that are not directly accessible */
	/* PCON bits */
	#define IDL             0x01
	#define PD              0x02
	#define GF0             0x04
	#define GF1             0x08
	#define SMOD            0x80

	/* TMOD bits */
	#define T0_M0           0x01
	#define T0_M1           0x02
	#define T0_CT           0x04
	#define T0_GATE         0x08
	#define T1_M0           0x10
	#define T1_M1           0x20
	#define T1_CT           0x40
	#define T1_GATE         0x80

	#define T0_MASK         0x0F
	#define T1_MASK         0xF0

	/* Interrupt numbers: address = (number * 8) + 3 */
	#define IE0_VECTOR      0       /* 0x03 external interrupt 0 */
	#define TF0_VECTOR      1       /* 0x0b timer 0 */
	#define IE1_VECTOR      2       /* 0x13 external interrupt 1 */
	#define TF1_VECTOR      3       /* 0x1b timer 1 */
	#define SI0_VECTOR      4       /* 0x23 serial port 0 */

#endif
