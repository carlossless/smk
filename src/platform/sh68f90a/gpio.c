#include "gpio.h"
#include "sh68f90a.h"
#include <stdint.h>

/*
# Rows
P7_1 - R0
P7_2 - R1
P7_3 - R2
P5_3 - R3
P5_4 - R4

# Columns
P5_0 - C0
P5_1 - C1
P5_2 - C2
P3_5 - C3
P3_4 - C4
P3_3 - C5
P3_2 - C6
P3_1 - C7
P3_0 - C8
P2_5 - C9
P2_4 - C10
P2_3 - C11 (UL 0)
P2_2 - C12 (UL 1)
P2_1 - C13 (UL 2)
P2_0 - C14 (UL 3)
P1_5 - C15 (UL 4)

# LEDs
P0_4 - R0 R
P6_1 - R0 G
P0_3 - R0 B

P6_7 - R1 R
P6_2 - R1 G
P6_6 - R1 B

P0_2 - R2 R
P6_3 - R2 G
P5_7 - R2 B

P4_5 - R3 R
P6_4 - R3 G
P4_6 - R3 B

P4_4 - R4 R
P6_5 - R4 G
P4_3 - R4 B

P1_1 - UL R
P1_2 - UL G
P1_3 - UL B
*/

void gpio_init()
{
    P1CR = (uint8_t)(_P1_1 | _P1_2 | _P1_3 | _P1_4 | _P1_5);
    P2CR = (uint8_t)(_P2_0 | _P2_1 | _P2_2 | _P2_3 | _P2_4 | _P2_5);
    P3CR = (uint8_t)(_P3_0 | _P3_1 | _P3_2 | _P3_3 | _P3_4 | _P3_5);
    P5CR = (uint8_t)(_P5_0 | _P5_1 | _P5_2 | _P5_7);

    P1PCR = (uint8_t)(_P1_1 | _P1_2 | _P1_3 | _P1_4 | _P1_5);
    P2PCR = (uint8_t)(_P2_0 | _P2_1 | _P2_2 | _P2_3 | _P2_4 | _P2_5);
    P3PCR = (uint8_t)(_P3_0 | _P3_1 | _P3_2 | _P3_3 | _P3_4 | _P3_5);
    P5PCR = (uint8_t)(_P5_0 | _P5_1 | _P5_2 | _P5_3 | _P5_4 | _P5_5 | _P5_6 | _P5_7);
    P7PCR = (uint8_t)(_P7_1 | _P1_2 | _P1_3);

    // P1_1 - UL R
    // P1_2 - UL G
    // P1_3 - UL B
    // P5_7 - R1 B
    P1 = (uint8_t)(_P1_1 | _P1_2 | _P1_3 | _P1_4 | _P1_5);
    P2 = (uint8_t)(_P2_0 | _P2_1 | _P2_2 | _P2_3 | _P2_4 | _P2_5);
    P3 = (uint8_t)(_P4_0 | _P3_1 | _P3_2 | _P3_3 | _P3_4 | _P3_5);
    P5 = (uint8_t)(_P5_0 | _P5_1 | _P5_2 | _P5_7);
    P7 = (uint8_t)(_P7_1);

    // P6_1 - R0 G
    // P6_2 - R1 G
    // P6_3 - R2 G
    // P6_4 - R3 G
    // P6_5 - R4 G
    // P6_6 - R1 B
    // P6_7 - R1 R
    P6CR  = (uint8_t)(_P6_1 | _P6_2 | _P6_3 | _P6_4 | _P6_5 | _P6_6 | _P6_7);
    P6PCR = (uint8_t)(_P6_1 | _P6_2 | _P6_3 | _P6_4 | _P6_5 | _P6_6 | _P6_7);
    P6    = (uint8_t)(_P6_1 | _P6_2 | _P6_3 | _P6_4 | _P6_5 | _P6_6 | _P6_7);

    // P4_3 - R4 B
    // P4_4 - R4 R
    // P4_5 - R3 R
    // P4_6 - R3 B
    P4CR  = (uint8_t)(_P4_3 | _P4_4 | _P4_5 | _P4_6);
    P4PCR = (uint8_t)(_P4_3 | _P4_4 | _P4_5 | _P4_6);
    P4    = (uint8_t)(_P4_3 | _P4_4 | _P4_5 | _P4_6);

    // P0_2 - R2 R
    // P0_3 - R0 B
    // P0_4 - R0 R
    P0CR  = (uint8_t)(_P0_2 | _P0_3 | _P0_4);
    P0PCR = (uint8_t)(_P0_2 | _P0_3 | _P0_4);
    P0    = (uint8_t)(_P0_2 | _P0_3 | _P0_4);

    // configure driving capabilities
    DRVCON = 0x05; // allow P1 to be changed
    P1DRV  = 0x00; // 25mA

    DRVCON = 0x45; // allow P2 to be changed
    P2DRV  = 0x00; // 25mA

    DRVCON = 0x85; // allow P3 to be changed
    P3DRV  = 0x00; // 25mA

    DRVCON = 0xc5; // allow P5 to be changed
    P5DRV  = 0x00; // 25mA
}
