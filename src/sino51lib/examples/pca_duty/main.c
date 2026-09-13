// The PCA units as an 8-bit PWM bank. pca_init() brings every unit up with all its compare
// modules driving; a duty only reloads cleanly between pca_hold() and pca_release(), which is
// what the loop below does. Which registers exist is the part's pca_hw.h.

#include "clock.h"
#include "delay.h"
#include "ldo.h"
#include "pca.h"
#include "peripherals.h"
#include "watchdog.h"

void main(void)
{
    ldo_init();
    clock_init();
    peripherals_init();

    sfr_page_1();
    pca_init();

    for (uint8_t duty = 0;; duty++) {
        watchdog_kick();

        pca_hold();
        P0CPL0 = duty;
        P0CPH0 = 0;
        pca_release();

        delay_ms(20);
    }
}
