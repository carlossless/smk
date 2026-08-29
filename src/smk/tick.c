#include "tick.h"
#include "systick.h"
#include "matrix.h"
#include "indicators.h"
#include "keyboard.h"
#include "sleep.h"
#include <stdbool.h>
#include <stdint.h>

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
