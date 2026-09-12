#include "kbdef.h"
#include "user_matrix.h"

// the rows are on SFR page 1 and the columns on page 0; leaving page 1 latched would stop
// RSTSTAT being the watchdog kick.
#define WITH_PAGE_1(body)            \
    do {                             \
        uint8_t saved_page = INSCON; \
        sfr_page_1();                \
        body;                        \
        INSCON = saved_page;         \
    } while (0)

#define WITH_PAGE_0(body)            \
    do {                             \
        uint8_t saved_page = INSCON; \
        sfr_page_0();                \
        body;                        \
        INSCON = saved_page;         \
    } while (0)

const __code uint8_t kb_col_masks[MATRIX_COLS] = {KB_C0_P0_0, KB_C1_P0_1, KB_C2_P0_2, KB_C3_P0_3, KB_C4_P0_4, KB_C5_P0_5, KB_C6_P0_6, KB_C7_P0_7, KB_C8_P1_0, KB_C9_P1_1, KB_C10_P1_2, KB_C11_P1_3, KB_C12_P1_4, KB_C13_P1_5, KB_C14_P1_6, KB_C15_P1_7, KB_C16_P4_6, KB_C17_P4_7};

void user_matrix_cols_deselect_all(void)
{
    WITH_PAGE_0({
        P0 |= KB_C_P0_MASK;
        P1 |= KB_C_P1_MASK;
        P4 |= KB_C_P4_MASK;
    });
}

void user_matrix_col_select(uint8_t col)
{
    uint8_t mask = kb_col_masks[col];

    WITH_PAGE_0({
        if (col < KB_C_P1_FIRST) {
            P0 &= (uint8_t)~mask;
        } else if (col < KB_C_P4_FIRST) {
            P1 &= (uint8_t)~mask;
        } else {
            P4 &= (uint8_t)~mask;
        }
    });
}

void user_matrix_col_deselect(uint8_t col)
{
    uint8_t mask = kb_col_masks[col];

    WITH_PAGE_0({
        if (col < KB_C_P1_FIRST) {
            P0 |= mask;
        } else if (col < KB_C_P4_FIRST) {
            P1 |= mask;
        } else {
            P4 |= mask;
        }
    });
}

// the two bits above the six rows carry the EEPROM bus, so force them high: a low there
// would otherwise read as a pressed key.
#define KB_R_NONE (uint8_t)~KB_R_P5_MASK

// the page must be forced: this runs in the tick interrupt, and on page 0 this address is P0.
uint8_t user_matrix_read_rows(void)
{
    uint8_t rows;

    WITH_PAGE_1({ rows = (uint8_t)((P5 & KB_R_P5_MASK) | KB_R_NONE); });

    return rows;
}
