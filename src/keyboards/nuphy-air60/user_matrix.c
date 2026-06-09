#include "kbdef.h"
#include "user_matrix.h"

// Stock-faithful matrix scan helpers. The Air60 matrix has columns
// scattered across four ports and rows on two ports:
//
//   cols on P1.5 (KB_C15), P2.0..2.5 (KB_C14..C9), P3.0..3.5 (KB_C8..C3),
//   P5.0..5.2 (KB_C2..C0).
//   rows on P7.1..7.3 (KB_R0..R2), P5.3..5.4 (KB_R3, KB_R4).
//
// Columns are PWM-output multiplexed pins (PWM00..05 / PWM10..15 /
// PWM25 / PWM05 / PWM40..42). When the LED PWM channels are disabled
// the same pins act as GPIO and we use them to drive the matrix.

#define KB_C_P1_MASK (uint8_t)(KB_C15_P1_5)
#define KB_C_P2_MASK (uint8_t)(KB_C14_P2_0 | KB_C13_P2_1 | KB_C12_P2_2 | KB_C11_P2_3 | KB_C10_P2_4 | KB_C9_P2_5)
#define KB_C_P3_MASK (uint8_t)(KB_C8_P3_0 | KB_C7_P3_1 | KB_C6_P3_2 | KB_C5_P3_3 | KB_C4_P3_4 | KB_C3_P3_5)
#define KB_C_P5_MASK (uint8_t)(KB_C0_P5_0 | KB_C1_P5_1 | KB_C2_P5_2)

void user_matrix_cols_high_all(void)
{
    P1 |= KB_C_P1_MASK;
    P2 |= KB_C_P2_MASK;
    P3 |= KB_C_P3_MASK;
    P5 |= KB_C_P5_MASK;
}

// Column direction control. PxCR bit set = output, clear = input (high-Z);
// the column pins carry no pull-up (see user_init.c), so input is a true
// float. Mirrors stock: drive only during the sweep, release to high-Z at
// rest so a parked PWM column can't source. (PxCR shares bit positions with
// the Px data masks.)
void user_matrix_cols_output(void)
{
    P1CR |= KB_C_P1_MASK;
    P2CR |= KB_C_P2_MASK;
    P3CR |= KB_C_P3_MASK;
    P5CR |= KB_C_P5_MASK;
}

void user_matrix_cols_highz(void)
{
    // Clear pull-control (PxPCR) then direction (PxCR) per port, matching stock's
    // matrix-end release (P1PCR&=0xCF;P1CR&=0xCF; P2PCR&=0xC0;P2CR&=0xC0; ...).
    // The columns carry no pull-up (user_init.c doesn't set their PxPCR), so the
    // PCR clears are a no-op in practice — kept for stock parity and to guarantee
    // a true float even if a pull is ever enabled on these pins.
    P1PCR &= (uint8_t)~KB_C_P1_MASK;
    P1CR  &= (uint8_t)~KB_C_P1_MASK;
    P2PCR &= (uint8_t)~KB_C_P2_MASK;
    P2CR  &= (uint8_t)~KB_C_P2_MASK;
    P3PCR &= (uint8_t)~KB_C_P3_MASK;
    P3CR  &= (uint8_t)~KB_C_P3_MASK;
    P5PCR &= (uint8_t)~KB_C_P5_MASK;
    P5CR  &= (uint8_t)~KB_C_P5_MASK;
}

void user_matrix_col_low(uint8_t col)
{
    switch (col) {
        case 0:  KB_C0  = 0; break;
        case 1:  KB_C1  = 0; break;
        case 2:  KB_C2  = 0; break;
        case 3:  KB_C3  = 0; break;
        case 4:  KB_C4  = 0; break;
        case 5:  KB_C5  = 0; break;
        case 6:  KB_C6  = 0; break;
        case 7:  KB_C7  = 0; break;
        case 8:  KB_C8  = 0; break;
        case 9:  KB_C9  = 0; break;
        case 10: KB_C10 = 0; break;
        case 11: KB_C11 = 0; break;
        case 12: KB_C12 = 0; break;
        case 13: KB_C13 = 0; break;
        case 14: KB_C14 = 0; break;
        case 15: KB_C15 = 0; break;
    }
}

void user_matrix_col_high(uint8_t col)
{
    switch (col) {
        case 0:  KB_C0  = 1; break;
        case 1:  KB_C1  = 1; break;
        case 2:  KB_C2  = 1; break;
        case 3:  KB_C3  = 1; break;
        case 4:  KB_C4  = 1; break;
        case 5:  KB_C5  = 1; break;
        case 6:  KB_C6  = 1; break;
        case 7:  KB_C7  = 1; break;
        case 8:  KB_C8  = 1; break;
        case 9:  KB_C9  = 1; break;
        case 10: KB_C10 = 1; break;
        case 11: KB_C11 = 1; break;
        case 12: KB_C12 = 1; break;
        case 13: KB_C13 = 1; break;
        case 14: KB_C14 = 1; break;
        case 15: KB_C15 = 1; break;
    }
}

uint8_t user_matrix_read_rows(void)
{
    // Pack 5 row bits into a single byte:
    //   bit 0 ← P7.1 (KB_R0)
    //   bit 1 ← P7.2 (KB_R1)
    //   bit 2 ← P7.3 (KB_R2)
    //   bit 3 ← P5.3 (KB_R3)
    //   bit 4 ← P5.4 (KB_R4)
    //   bits 5..7 = 1 (tag — keeps "no rows pressed" reading == 0xFF)
    return (uint8_t)(((P7 >> 1) & 0x07) | (P5 & 0x18) | 0xE0);
}

void user_matrix_sinks_off(void)
{
    P0 &= ~(uint8_t)(RGB_R2R_P0_2 | RGB_R0B_P0_3 | RGB_R0R_P0_4);
    P1 &= ~(uint8_t)(RGB_ULR_P1_1 | RGB_ULG_P1_2 | RGB_ULB_P1_3);
    P4 &= ~(uint8_t)(RGB_R4B_P4_3 | RGB_R4R_P4_4 | RGB_R3R_P4_5 | RGB_R3B_P4_6);
    P5 &= ~(uint8_t)(RGB_R2B_P5_7);
    P6 &= ~(uint8_t)(RGB_R0G_P6_1 | RGB_R1G_P6_2 | RGB_R2G_P6_3 | RGB_R3G_P6_4 | RGB_R4G_P6_5 | RGB_R1B_P6_6 | RGB_R1R_P6_7);
}
