#include "timer2.h"
#include "matrix.h"
#include "indicators.h"
#include "keyboard.h"
#include "sleep.h"

// 16-bit auto-reload mode, T2 clock = SYS_CLK/12 = 2 MHz. Two periods, one per
// phase. The scan ISR body runs ~320 µs, so the short matrix reload overflows
// *during* the scan and the first LED subframe fires the instant it returns
// (instead of idling out a full 400 µs period, leaving LEDs dark longer). Each
// phase reloads at entry, so every LED subframe still gets a full 400 µs.
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

// Matrix scans are interleaved with the LED PWM subframes on this one timer. A
// scan runs after every LED_SUBFRAMES_PER_SCAN LED subframes: 1 maximises the key
// scan rate (a scan between every subframe, ~950 Hz — near the 1 kHz USB poll
// ceiling), a larger value trades scan rate back for LED brightness/smoothness
// (the backlight is blanked for the ~0.66 ms of each scan). The full LED frame
// still takes LED_SCAN_ROWS*3 subframes; interleaving just stretches it in time.
#define LED_SUBFRAMES_PER_SCAN 1

// Phase: 0 = next fire runs a matrix scan, 1 = next runs an LED substep.
static volatile uint8_t phase          = 0;
static volatile uint8_t led_since_scan = 0;

void timer2_interrupt_handler(void) __interrupt(_INT_TIMER2)
{
    TF2 = 0;

    if (phase == 0) {
        phase          = 1;
        led_since_scan = 0;
        // Short reload overflows during the scan so the first LED subframe
        // resumes the instant matrix_scan_full() returns.
        timer2_reload(T2_RELOAD_MATRIX);
        matrix_scan_full();
    } else {
        timer2_reload(T2_RELOAD_LED); // full 400 µs dwell

        indicators_pre_update();
        const bool frame_wrapped = indicators_update_step(&keyboard_state, 0);
        indicators_post_update();

        // Idle timer runs off the LED frame, not the (now much faster) scan, so
        // its cadence stays bounded regardless of LED_SUBFRAMES_PER_SCAN.
        if (frame_wrapped) {
            sleep_tick();
        }

        if (++led_since_scan >= LED_SUBFRAMES_PER_SCAN) {
            phase = 0;
        }
    }
}
