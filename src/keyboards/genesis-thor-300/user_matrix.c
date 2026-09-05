#include "kbdef.h"
#include "user_matrix.h"

// The column ports live on SFR page 1 and the row ports on page 0, so every column
// access borrows page 1 and hands it straight back. Leaving page 1 latched would stop
// RSTSTAT being the watchdog kick and the part would reset in a loop.
#define WITH_COLUMN_PAGE(body)       \
    do {                             \
        uint8_t saved_page = INSCON; \
        sfr_page_1();                \
        body;                        \
        INSCON = saved_page;         \
    } while (0)

const __code uint8_t kb_col_pins[MATRIX_COLS] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 23,
};

void user_matrix_cols_deselect_all(void)
{
    WITH_COLUMN_PAGE({
        P6 |= KB_C_P6_MASK;
        P7 |= KB_C_P7_MASK;
        P8 |= KB_C_P8_MASK;
    });
}

void user_matrix_col_select(uint8_t col)
{
    uint8_t pin = kb_col_pins[col];

    WITH_COLUMN_PAGE({
        if (pin < 8) {
            P6 &= (uint8_t)~(1u << pin);
        } else if (pin < 16) {
            P7 &= (uint8_t)~(1u << (pin - 8));
        } else {
            P8 &= (uint8_t)~(1u << (pin - 16));
        }
    });
}

void user_matrix_col_deselect(uint8_t col)
{
    uint8_t pin = kb_col_pins[col];

    WITH_COLUMN_PAGE({
        if (pin < 8) {
            P6 |= (uint8_t)(1u << pin);
        } else if (pin < 16) {
            P7 |= (uint8_t)(1u << (pin - 8));
        } else {
            P8 |= (uint8_t)(1u << (pin - 16));
        }
    });
}

// R0-R2 are P2.1-P2.3 and R3-R5 are P4.4-P4.6, transcribed from the stock scan at
// CODE:0x4f62. The unused top two bits are forced high so they never look pressed.
//
// The page must be forced, not assumed: the scan runs in the tick interrupt, which can
// land on main-loop code that is inside a page-1 window. On page 1 these addresses are
// P7 and EP1CON, so the rows would be read out of the USB block.
uint8_t user_matrix_read_rows(void)
{
    uint8_t rows;

    uint8_t saved_page = INSCON;
    sfr_page_0();
    rows   = (uint8_t)(((P2 >> 1) & 0x07u) | ((P4 >> 1) & 0x38u) | 0xC0u);
    INSCON = saved_page;

    return rows;
}
