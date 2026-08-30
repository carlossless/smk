#include "kbdef.h"
#include "gpio.h"
#include "user_matrix.h"

#define KB_C_P1_MASK (uint8_t)(KB_C15_P1_5)
#define KB_C_P2_MASK (uint8_t)(KB_C14_P2_0 | KB_C13_P2_1 | KB_C12_P2_2 | KB_C11_P2_3 | KB_C10_P2_4 | KB_C9_P2_5)
#define KB_C_P3_MASK (uint8_t)(KB_C8_P3_0 | KB_C7_P3_1 | KB_C6_P3_2 | KB_C5_P3_3 | KB_C4_P3_4 | KB_C3_P3_5)
#define KB_C_P5_MASK (uint8_t)(KB_C0_P5_0 | KB_C1_P5_1 | KB_C2_P5_2)

void user_matrix_cols_deselect_all(void)
{
    GPIO_HIGH(1, KB_C_P1_MASK);
    GPIO_HIGH(2, KB_C_P2_MASK);
    GPIO_HIGH(3, KB_C_P3_MASK);
    GPIO_HIGH(5, KB_C_P5_MASK);
}

void user_matrix_scan_pre(void)
{
    GPIO_OUTPUT(1, KB_C_P1_MASK);
    GPIO_OUTPUT(2, KB_C_P2_MASK);
    GPIO_OUTPUT(3, KB_C_P3_MASK);
    GPIO_OUTPUT(5, KB_C_P5_MASK);
}

void user_matrix_scan_post(void)
{
    GPIO_PULLUP_OFF(1, KB_C_P1_MASK);
    GPIO_INPUT(1, KB_C_P1_MASK);
    GPIO_PULLUP_OFF(2, KB_C_P2_MASK);
    GPIO_INPUT(2, KB_C_P2_MASK);
    GPIO_PULLUP_OFF(3, KB_C_P3_MASK);
    GPIO_INPUT(3, KB_C_P3_MASK);
    GPIO_PULLUP_OFF(5, KB_C_P5_MASK);
    GPIO_INPUT(5, KB_C_P5_MASK);
}

void user_matrix_col_select(uint8_t col) // active-low: drive LOW
{
    switch (col) {
        case 0:
            KB_C0 = 0;
            break;
        case 1:
            KB_C1 = 0;
            break;
        case 2:
            KB_C2 = 0;
            break;
        case 3:
            KB_C3 = 0;
            break;
        case 4:
            KB_C4 = 0;
            break;
        case 5:
            KB_C5 = 0;
            break;
        case 6:
            KB_C6 = 0;
            break;
        case 7:
            KB_C7 = 0;
            break;
        case 8:
            KB_C8 = 0;
            break;
        case 9:
            KB_C9 = 0;
            break;
        case 10:
            KB_C10 = 0;
            break;
        case 11:
            KB_C11 = 0;
            break;
        case 12:
            KB_C12 = 0;
            break;
        case 13:
            KB_C13 = 0;
            break;
        case 14:
            KB_C14 = 0;
            break;
        case 15:
            KB_C15 = 0;
            break;
    }
}

void user_matrix_col_deselect(uint8_t col) // active-low: drive HIGH (idle)
{
    switch (col) {
        case 0:
            KB_C0 = 1;
            break;
        case 1:
            KB_C1 = 1;
            break;
        case 2:
            KB_C2 = 1;
            break;
        case 3:
            KB_C3 = 1;
            break;
        case 4:
            KB_C4 = 1;
            break;
        case 5:
            KB_C5 = 1;
            break;
        case 6:
            KB_C6 = 1;
            break;
        case 7:
            KB_C7 = 1;
            break;
        case 8:
            KB_C8 = 1;
            break;
        case 9:
            KB_C9 = 1;
            break;
        case 10:
            KB_C10 = 1;
            break;
        case 11:
            KB_C11 = 1;
            break;
        case 12:
            KB_C12 = 1;
            break;
        case 13:
            KB_C13 = 1;
            break;
        case 14:
            KB_C14 = 1;
            break;
        case 15:
            KB_C15 = 1;
            break;
    }
}

uint8_t user_matrix_read_rows(void)
{
    return (uint8_t)(((P7 >> 1) & 0x07) | (P5 & 0x18) | 0xE0);
}

void user_matrix_sinks_off(void)
{
    GPIO_LOW(0, RGB_R2R_P0_2 | RGB_R0B_P0_3 | RGB_R0R_P0_4);
    GPIO_LOW(1, RGB_ULR_P1_1 | RGB_ULG_P1_2 | RGB_ULB_P1_3);
    GPIO_LOW(4, RGB_R4B_P4_3 | RGB_R4R_P4_4 | RGB_R3R_P4_5 | RGB_R3B_P4_6);
    GPIO_LOW(5, RGB_R2B_P5_7);
    GPIO_LOW(6, RGB_R0G_P6_1 | RGB_R1G_P6_2 | RGB_R2G_P6_3 | RGB_R3G_P6_4 | RGB_R4G_P6_5 | RGB_R1B_P6_6 | RGB_R1R_P6_7);
}
