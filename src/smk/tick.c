#include "tick.h"
#include "systick.h"
#include "matrix.h"
#include "indicators.h"
#include "keyboard.h"
#include "sleep.h"
#include <stdbool.h>
#include <stdint.h>

// LED subframes emitted between two matrix sweeps. 1 maximises the key scan rate
// (~950 Hz, just under the 1 kHz USB poll ceiling) and costs backlight duty,
// since the LEDs are blanked for the ~0.66 ms each sweep takes; raising it
// trades scan rate back for brightness. A full LED frame is LED_SCAN_ROWS*3
// subframes either way — interleaving only stretches it in time.
#define LED_SUBFRAMES_PER_SCAN 1

static volatile bool    scan_due;
static volatile uint8_t subframes_since_scan;

void tick_init(void)
{
    scan_due             = true;
    subframes_since_scan = 0;

    systick_init();
}

static void run_matrix_scan(void)
{
    systick_arm(SYSTICK_SLOT_MATRIX_SCAN);
    matrix_scan_full();
}

static void run_led_subframe(void)
{
    systick_arm(SYSTICK_SLOT_LED_SUBFRAME);

    indicators_pre_update();
    const bool frame_wrapped = indicators_update_step(&keyboard_state, 0);
    indicators_post_update();

    sleep_note_frame(frame_wrapped);
}

void tick_dispatch(void)
{
    if (scan_due) {
        scan_due             = false;
        subframes_since_scan = 0;
        run_matrix_scan();
        return;
    }

    run_led_subframe();

    if (++subframes_since_scan >= LED_SUBFRAMES_PER_SCAN) {
        scan_due = true;
    }
}

void tick_pause(void)
{
    systick_pause();
}

void tick_resume(void)
{
    systick_resume();
}
