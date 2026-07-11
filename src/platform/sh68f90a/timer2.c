#include "timer2.h"
#include "matrix.h"
#include "indicators.h"
#include "keyboard.h"
#include "sleep.h"

// 16-bit auto-reload mode. T2 clock = SYS_CLK/12 = 24/12 = 2 MHz.
//
// Two periods, one per phase:
//   - LED subframe: 0xFCDF → 801 counts → 400 µs dwell, the per-subframe LED
//     display time.
//   - matrix scan: 0xFF37 → ~100 µs. The matrix scan ISR body runs ~320 µs, so
//     this short reload overflows *during* the scan; the pending interrupt then
//     fires the first LED subframe the instant the scan returns, instead of
//     idling out the rest of a full 400 µs period (which left the LEDs dark
//     ~80 µs longer per frame). Each phase reloads the timer at entry, so every
//     LED subframe still gets a full fresh 400 µs.
#define T2_RELOAD_LED    0xFCDF // = 65536 - 801, 400 µs
#define T2_RELOAD_MATRIX 0xFF37 // = 65536 - 201, ~100 µs

static void timer2_reload(uint16_t reload)
{
    TR2    = 0;
    RCAP2H = (uint8_t)(reload >> 8);
    RCAP2L = (uint8_t)(reload & 0xFF);
    TH2    = (uint8_t)(reload >> 8);
    TL2    = (uint8_t)(reload & 0xFF);
    TR2    = 1;
}

void timer2_init(void)
{
    TR2   = 0;
    T2CON = 0;
    T2MOD = 0;
    timer2_reload(T2_RELOAD_LED);
    TF2 = 0;
    ET2 = 1;
    TR2 = 1;
}

void timer2_scan_pause(void)
{
    ET2 = 0;
}

void timer2_scan_resume(void)
{
    ET2 = 1;
}

// Phase: 0 = next fire runs a full matrix scan, 1 = next fire runs an LED
// substep. The LED substep counter is owned by the indicators module; we just
// call its step function and let it advance. When the LED phase wraps a full
// frame, indicators_update_step returns true and we flip back to phase 0.
static volatile uint8_t phase = 0;

void timer2_interrupt_handler(void) __interrupt(_INT_TIMER2)
{
    TF2 = 0;

    if (phase == 0) {
        phase = 1;
        // Short reload: overflows during the long scan so the first LED
        // subframe resumes the instant matrix_scan_full() returns.
        timer2_reload(T2_RELOAD_MATRIX);
        matrix_scan_full();

        // Advance the inactivity counter once per matrix frame. Cheap and a no-op
        // until the threshold; see src/smk/sleep.c.
        sleep_tick();
    } else {
        // Full 400 µs dwell for this LED subframe.
        timer2_reload(T2_RELOAD_LED);
        indicators_pre_update();
        const bool frame_wrapped = indicators_update_step(&keyboard_state, 0);
        indicators_post_update();
        if (frame_wrapped) {
            phase = 0;
        }
    }
}
