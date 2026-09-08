#include "kbdef.h"
#include "user_matrix.h"

// column ports are on SFR page 1, rows on page 0; leaving page 1 latched would stop RSTSTAT being the watchdog kick.
#define WITH_COLUMN_PAGE(body)       \
    do {                             \
        uint8_t saved_page = INSCON; \
        sfr_page_1();                \
        body;                        \
        INSCON = saved_page;         \
    } while (0)

const __code uint8_t kb_col_masks[MATRIX_COLS] = {KB_C0_P6_1, KB_C1_P6_2, KB_C2_P6_3, KB_C3_P6_4, KB_C4_P6_5, KB_C5_P6_6, KB_C6_P6_7, KB_C7_P7_0, KB_C8_P7_1, KB_C9_P7_2, KB_C10_P7_3, KB_C11_P7_4, KB_C12_P7_5, KB_C13_P7_6, KB_C14_P7_7, KB_C15_P8_0, KB_C16_P8_1, KB_C17_P8_2, KB_C18_P8_7};

void user_matrix_cols_deselect_all(void)
{
    WITH_COLUMN_PAGE({
        P6 |= _P6_ALL;
        P7 |= _P7_ALL;
        P8 |= _P8_ALL;
    });
}

void user_matrix_col_select(uint8_t col)
{
    uint8_t mask = kb_col_masks[col];

    WITH_COLUMN_PAGE({
        if (col < KB_C_P7_FIRST) {
            P6 &= (uint8_t)~mask;
        } else if (col < KB_C_P8_FIRST) {
            P7 &= (uint8_t)~mask;
        } else {
            P8 &= (uint8_t)~mask;
        }
    });
}

void user_matrix_col_deselect(uint8_t col)
{
    uint8_t mask = kb_col_masks[col];

    WITH_COLUMN_PAGE({
        if (col < KB_C_P7_FIRST) {
            P6 |= mask;
        } else if (col < KB_C_P8_FIRST) {
            P7 |= mask;
        } else {
            P8 |= mask;
        }
    });
}

// the bits above the six rows are forced high so they can never look pressed.
#define KB_R_NONE (uint8_t)~((KB_R_P2_MASK | KB_R_P4_MASK) >> 1)

// the page must be forced: this runs in the tick interrupt, and on page 1 these addresses are P7 and EP1CON in the USB block.
uint8_t user_matrix_read_rows(void)
{
    uint8_t rows;

    uint8_t saved_page = INSCON;
    sfr_page_0();
    rows = (uint8_t)(((P2 & KB_R_P2_MASK) >> 1) | ((P4 & KB_R_P4_MASK) >> 1) | KB_R_NONE);

    INSCON = saved_page;

    return rows;
}
