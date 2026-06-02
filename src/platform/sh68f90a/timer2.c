#include "timer2.h"
#include "matrix.h"
#include "indicators.h"
#include "keyboard.h"

// 16-bit auto-reload mode. T2 clock = SYS_CLK/12 = 24/12 = 2 MHz.
// Stock-match reload = 0xFCDF → 801 counts → 400 µs period (2.5 kHz fire
// rate). This MUST be longer than the matrix-scan ISR body (~350 µs at
// 10 µs settle × 2 samples × 16 cols + PWM enable/disable overhead),
// otherwise the matrix scan eats into the next Timer 2 period and the
// first LED substep that follows gets a shortened window — visible as
// the top row going dim during animations.
#define T2_RELOAD 0xFCDF // = 65536 - 801, 400 µs at T2_clk = 2 MHz

void timer2_init(void)
{
    TR2 = 0;
    T2CON  = 0;
    T2MOD  = 0;
    RCAP2H = (uint8_t)(T2_RELOAD >> 8);
    RCAP2L = (uint8_t)(T2_RELOAD & 0xFF);
    TH2    = (uint8_t)(T2_RELOAD >> 8);
    TL2    = (uint8_t)(T2_RELOAD & 0xFF);
    TF2 = 0;
    ET2 = 1;
    TR2 = 1;
}

// Phase: 0 = next fire runs a full matrix scan, 1 = next fire runs an LED
// substep. The LED substep counter (anim_tick_counter) is owned by the
// indicators module; we just call its step function and let it advance.
// When the LED phase wraps a full frame, indicators_update_step returns
// true and we flip back to phase 0 — exactly the stock dance.
static volatile uint8_t phase = 0;

void timer2_interrupt_handler(void) __interrupt(_INT_TIMER2)
{
    TF2 = 0;

    if (phase == 0) {
        phase = 1;
        matrix_scan_full();
    } else {
        indicators_pre_update();
        const bool frame_wrapped = indicators_update_step(&keyboard_state, 0);
        indicators_post_update();
        if (frame_wrapped) {
            phase = 0;
        }
    }
}
