#include "timer2.h"
#include "matrix.h"
#include "indicators.h"
#include "keyboard.h"
#include "sleep.h"

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

static volatile uint8_t phase = 0;

void timer2_interrupt_handler(void) __interrupt(_INT_TIMER2)
{
    TF2 = 0;

    if (phase == 0) {
        phase = 1;
        timer2_reload(T2_RELOAD_MATRIX);
        matrix_scan_full();

        sleep_tick();
    } else {
        timer2_reload(T2_RELOAD_LED);
        indicators_pre_update();
        const bool frame_wrapped = indicators_update_step(&keyboard_state, 0);
        indicators_post_update();
        if (frame_wrapped) {
            phase = 0;
        }
    }
}
