#include "sleep.h"

#ifndef SLEEP_ENABLE
// sleep.h collapses to macros when the feature is off, leaving this an empty
// translation unit — which ISO C forbids (--std-c23 + --Werror rejects it).
// One unused typedef keeps the TU non-empty and emits nothing.
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

// Inactivity threshold, in matrix frames. We tick once per matrix frame
// (sleep_tick from the Timer 2 ISR) — a frame is ~8.7 ms (1 matrix scan + ~21
// LED subframes).
//
// Real-world idle timeout: ~6 minutes. At ~8.7 ms/frame that's
// 6 * 60 / 0.0087 ≈ 41,400 frames (well within the 16-bit counter's 65535 cap).
// Any key wakes it.
#    define SLEEP_TIMEOUT 41400

// All state lives in __xdata — the SH68F90A is at its internal-RAM ceiling, so
// new state must not land in the default (internal) segment.
//
// `inactivity` is owned by the Timer 2 ISR (sleep_tick) during normal running;
// the main loop only ever reads it indirectly via `sleep_requested`. Activity
// is communicated to the ISR through the single-byte `activity_seen` flag, so
// there is no read-modify-write race on the 16-bit counter across contexts.
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
    // A key changed since the last tick — reset and start counting again, and
    // drop any pending request that hadn't been consumed yet.
    if (activity_seen) {
        activity_seen   = 0;
        inactivity      = 0;
        sleep_requested = 0;
        return;
    }

    // Climb to the threshold and latch the request once, then stop counting so
    // the 16-bit value can't wrap back around past the threshold.
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
    // Which sleep variant applies right now (tracks the conn slider live).
    const user_sleep_mode_t mode = user_sleep_supported();

    // Decide whether to sleep, per mode:
    //  - RF: inactivity timeout (sleep_requested, latched by sleep_tick).
    //  - USB: only once the *host* has suspended the bus (usb_suspended). The
    //    inactivity timer is irrelevant here — self-suspending a host that's
    //    still polling would break the link.
    bool go = false;
    if (mode == USER_SLEEP_RF) {
        go = sleep_requested;
    } else if (mode == USER_SLEEP_USB) {
        go = usb_suspended;
    }

    if (!go) {
        // Outside RF mode the inactivity counter is irrelevant (USB sleeps on
        // host-suspend, not this timer). Park it there — keep it zeroed — so it
        // can't quietly climb to the threshold and latch a sleep request that
        // would then fire the instant the slider flips to RF. In RF mode, leave
        // it alone so it keeps climbing toward the timeout. (activity_seen is the
        // ISR-safe single-byte handshake; sleep_tick zeroes the counter + clears
        // the request on the next tick.)
        if (mode != USER_SLEEP_RF) {
            activity_seen = 1;
        }
        return;
    }

    sleep_requested = 0; // consume the latch now that we're about to sleep

    const bool usb_mode = (mode == USER_SLEEP_USB);

#    ifdef RF_ENABLED
    // BK3632 sleep handshake: in RF mode tell the radio the MCU is going down so
    // it stays awake and can wake us via its ACK line (P4.2 / INT42), alongside
    // keypress wake (P4.1). Not needed in USB mode (the radio isn't the active
    // link).
    if (!usb_mode) {
        rf_wake_from_sleep();
    }
#    endif

    // Stop the LED/matrix scan ISR and park the PWM (LEDs dark).
    timer2_scan_pause();
    indicators_pwm_disable();

    // Board hook: park every port into its low-power state and arm the INT4 wake
    // (covers P4.1 keypress + P4.2 BK3632). Then drop into Power-Down — the core
    // halts inside power_enter_powerdown() until a wake fires (INT4, or a USB bus
    // resume in the USB-suspend variant).
    user_sleep_prepare();
    power_enter_powerdown(usb_mode);
    user_sleep_wake(); // restore GPIO + RF MOT pin, disable the wake int

    // Bring the visible subsystems back.
    indicators_pwm_enable();
    timer2_scan_resume();

#    ifdef RF_ENABLED
    // Re-sync the BK3632 after an RF-mode wake: nudge + status-poll up to 10×,
    // until the link reports connected again. The loop counter lives in __xdata
    // (XRAM), not on the internal-RAM-overlaid stack, so this heavier path
    // doesn't blow the IRAM ceiling.
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

    // Whatever woke us (a keypress, usually) counts as activity; let the next
    // Timer 2 tick zero the counter so we don't immediately re-arm.
    activity_seen = 1;
}

#endif // SLEEP_ENABLE
