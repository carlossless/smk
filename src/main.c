#include "clock.h"
#include "ldo.h"
#include "watchdog.h"
#include "delay.h"
#include "isp.h"
#ifdef DEBUG_SINK_UART
#    include "uart.h"
#endif
#include "usb.h"
#include "debug.h"
#include "console.h"
#include "matrix.h"
#include "utils.h"
#include "keyboard.h"
#include "user_init.h"
#include "indicators.h"
#include "kb.h"
#include "stack.h"
#include "settings.h"
#ifdef RF_ENABLED
#    include "rf_controller.h"
#endif

#include "pwm.h"    // TODO: centralise interrupt definitions
#include "timer2.h" // ISR vector slot must be visible where main() is compiled
#include "sleep.h"
#include "power.h" // int4_isr ISR vector slot — same reason

void init()
{
    ldo_init();
    clock_init();
#ifdef DEBUG_SINK_UART
    uart_init();
#endif

    user_init();

    matrix_init();
    keyboard_init();
    usb_init();

    // Zero the LED framebuffers before EA=1 starts the scan ISR — cold boots
    // don't guarantee cleared xdata, and the scan would stream garbage until
    // the first render.
    indicators_init();

    // Timer 2 drives the matrix scan + LED PWM in a single alternating ISR.
    timer2_init();

    EA = 1;
}

void main()
{
    init();

#if DEBUG == 1
    // Paint the unused stack with a sentinel for stack_task()'s high-water
    // measurement. Must run after init() — anything before clock_init() bricks
    // the keyboard.
    stack_paint();
#endif

    dprintf("SMK v" TOSTRING(SMK_VERSION) "\r\n");
    dprintf("KB " KEYBOARD_NAME " / " LAYOUT_NAME "\r\n");
    dprintf("DEVICE vId:" TOSTRING(USB_VID) " pId:" TOSTRING(USB_PID) "\n\r");

    kb_init();

#ifdef RF_ENABLED
    rf_init();
#endif

    // Load user_settings from flash; if the record fails its magic/length/
    // checksum check, seed factory defaults. Either way, clamp out-of-range
    // LED fields.
    if (!settings_load()) {
        indicators_apply_defaults();
    }
    indicators_validate_settings();
#if DEBUG == 1
    settings_dump();
#endif

    // Hold the backlight dark until USB finishes enumerating: enumeration
    // control-transfer traffic starves the LED scan, over-brightening whatever
    // row is mid-scan (the "boot blip"). delay_ms busy-waits with interrupts on,
    // so the scan (dark, framebuffer zeroed) and USB ISRs keep running.
    //   - USB present: usb_enum_seen latches on the first SETUP; wait for
    //     usb_enum_active_ticks to hit 0 (~500 ms quiet = enum + HID attach done).
    //   - Battery / power-only: no SETUP ever — light up after a short window.
    //   - Hard cap so a pathological host can't keep the backlight off forever.
    for (uint16_t i = 0; i < 4000; i++) {
        CLR_WDT();
        if (usb_enum_seen) {
            if (usb_enum_active_ticks == 0) {
                break; // enumerated and gone quiet
            }
        } else if (i >= 500) {
            break; // no enumeration seen — not on a USB host
        }
        delay_ms(1);
    }

    indicators_start();

#ifdef RF_ENABLED
    // Re-establish the last link the user was on.
    rf_set_link((rf_mode_t)user_settings.rf_link);
    // Optimistically prime keyboard_state to the commanded link so the indicator
    // shows the right colour immediately, instead of the default + unpaired-blink
    // window while the first status poll lags. The supervisor downgrades these
    // if the BK3632 disagrees.
    keyboard_state.rf_link   = user_settings.rf_link;
    keyboard_state.connected = 1;
    keyboard_state.paired    = 1;
#endif

    // Must run after the board's GPIO/RF are up.
    sleep_init();

    while (1) {
        CLR_WDT();

        kb_update_switches();
        kb_update();
        matrix_task();

        // Regenerate the effect framebuffer here, out of the scan ISR (which
        // only streams it to the PWM). Animation phase is clocked in the ISR, so
        // this free-runs at the loop rate without affecting animation speed.
        indicators_render();

        // Deferred settings save: handlers flip a dirty bit; the flush happens
        // here once per iteration, coalescing rapid changes into one erase +
        // program so the ~5 ms erase stall doesn't fire on every keypress.
        settings_task();

        // Inactivity sleep: on timeout, drops the MCU into Power-Down and blocks
        // until a keypress (INT4) or USB event wakes it. No-op otherwise.
        sleep_task();

#if DEBUG == 1
        stack_task();
        console_task();
#endif
    }
}
