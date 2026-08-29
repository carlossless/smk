#include "sleep.h"

#ifndef SLEEP_ENABLE
// With the feature off, sleep.h is all macros and this file would be an empty
// translation unit (which ISO C forbids). One unused typedef keeps it non-empty.
typedef int sleep_disabled_placeholder_t;
#else

#    include "indicators.h"
#    include "tick.h"
#    include "power.h"
#    include "usb.h"
#    include "user_sleep.h"
#    ifdef RF_ENABLED
#        include "rf_controller.h"
#        include "keyboard.h"
#        include "delay.h"
#    endif

#    include <stdbool.h>
#    include <stdint.h>

// Inactivity threshold, in LED frames. A frame completes once per LED sweep
// (~59 Hz with LED_SUBFRAMES_PER_SCAN=1), so ~21,000 frames is roughly a
// 6-minute idle timeout. Retune if the LED frame rate changes (see tick.c).
#    define SLEEP_TIMEOUT 21000

// `inactivity` is owned by sleep_note_frame(); the main loop only ever reads it
// indirectly via `sleep_requested`. Activity reaches the tick through the
// single-byte `activity_seen` flag, so there is no read-modify-write race on the
// 16-bit counter across contexts.
static volatile uint16_t inactivity;
static volatile uint8_t  activity_seen;
static volatile uint8_t  sleep_requested;

void sleep_init(void)
{
    inactivity      = 0;
    activity_seen   = 0;
    sleep_requested = 0;
}

void sleep_note_frame(bool frame_completed)
{
    if (!frame_completed) {
        return;
    }

    if (activity_seen) {
        activity_seen   = 0;
        inactivity      = 0;
        sleep_requested = 0;
        return;
    }

    // Stop counting once the request is latched, so the 16-bit value can't wrap
    // back around past the threshold.
    if (inactivity < SLEEP_TIMEOUT) {
        if (++inactivity >= SLEEP_TIMEOUT) {
            sleep_requested = 1;
        }
    }
}

void sleep_note_activity(void)
{
    activity_seen = 1;
}

static bool sleep_due(user_sleep_mode_t mode)
{
    switch (mode) {
        case USER_SLEEP_RF:
            return sleep_requested;
        // On USB the host decides. Self-suspending a bus it is still polling
        // would break the link, so the inactivity timer has no say here.
        case USER_SLEEP_USB:
            return usb_suspended;
        default:
            return false;
    }
}

#    ifdef RF_ENABLED
#        define RF_RESYNC_TRIES 10

static void rf_resync_after_wake(void)
{
    static uint8_t tries;

    for (tries = RF_RESYNC_TRIES; tries > 0; tries--) {
        rf_wake_nudge();
        delay_ms(10);
        if (rf_update_keyboard_state(&keyboard_state) && keyboard_state.connected) {
            return;
        }
        delay_ms(10);
    }
}
#    endif

void sleep_task(void)
{
    const user_sleep_mode_t mode = user_sleep_supported(); // tracks the conn slider live

    if (!sleep_due(mode)) {
        // Outside RF mode the inactivity counter has no job, so park it at zero.
        // Left to climb it would latch a request that then fires the instant the
        // slider flips to RF.
        if (mode != USER_SLEEP_RF) {
            sleep_note_activity();
        }
        return;
    }

    sleep_requested = 0;

    const powerdown_mode_t powerdown = (mode == USER_SLEEP_USB) ? POWERDOWN_KEEP_USB_ALIVE : POWERDOWN_RELEASE_USB;

#    ifdef RF_ENABLED
    // Tell the radio the MCU is going down so it stays awake and can wake us.
    // Not needed on USB, where the radio isn't the active link.
    if (powerdown == POWERDOWN_RELEASE_USB) {
        rf_wake_from_sleep();
    }
#    endif

    tick_pause();
    indicators_pwm_disable();

    user_sleep_prepare(); // parks the board's hardware and arms the wake source
    power_enter_powerdown(powerdown);
    user_sleep_wake();

    indicators_pwm_enable();
    tick_resume();

#    ifdef RF_ENABLED
    if (powerdown == POWERDOWN_RELEASE_USB) {
        rf_resync_after_wake();
    }
#    endif

    // Whatever woke us counts as activity; the next tick zeroes the counter so
    // we don't immediately re-arm.
    sleep_note_activity();
}

#endif // SLEEP_ENABLE
