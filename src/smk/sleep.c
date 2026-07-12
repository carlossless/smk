#include "sleep.h"

#ifndef SLEEP_ENABLE
typedef int sleep_disabled_placeholder_t;
#else

#    include "indicators.h"
#    include "timer2.h"
#    include "power.h"
#    include "usb.h"
#    include "user_sleep.h"
#    ifdef RF_ENABLED
#        include "rf_controller.h"
#        include "keyboard.h"
#        include "delay.h"
#    endif

#    include <stdint.h>

#    define SLEEP_TIMEOUT 21000

static volatile __xdata uint16_t inactivity;
static volatile __xdata uint8_t  activity_seen;
static volatile __xdata uint8_t  sleep_requested;

void sleep_init(void)
{
    inactivity      = 0;
    activity_seen   = 0;
    sleep_requested = 0;
}

void sleep_tick(void)
{
    if (activity_seen) {
        activity_seen   = 0;
        inactivity      = 0;
        sleep_requested = 0;
        return;
    }

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

void sleep_task(void)
{
    const user_sleep_mode_t mode = user_sleep_supported();

    bool go = false;
    if (mode == USER_SLEEP_RF) {
        go = sleep_requested;
    } else if (mode == USER_SLEEP_USB) {
        go = usb_suspended;
    }

    if (!go) {
        if (mode != USER_SLEEP_RF) {
            activity_seen = 1;
        }
        return;
    }

    sleep_requested = 0; // consume the latch now that we're about to sleep

    const bool usb_mode = (mode == USER_SLEEP_USB);

#    ifdef RF_ENABLED
    if (!usb_mode) {
        rf_wake_from_sleep();
    }
#    endif

    timer2_scan_pause();
    indicators_pwm_disable();

    user_sleep_prepare();
    power_enter_powerdown(usb_mode);
    user_sleep_wake();

    indicators_pwm_enable();
    timer2_scan_resume();

#    ifdef RF_ENABLED
    if (!usb_mode) {
        static __xdata uint8_t resync_tries;
        for (resync_tries = 10; resync_tries > 0; resync_tries--) {
            rf_wake_nudge();
            delay_ms(10);
            if (rf_update_keyboard_state(&keyboard_state) && keyboard_state.connected) {
                break;
            }
            delay_ms(10);
        }
    }
#    endif

    activity_seen = 1;
}

#endif // SLEEP_ENABLE
