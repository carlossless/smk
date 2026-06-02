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

#include "pwm.h"    // TODO: interrupt is defined here and need to be imported in main, centralise interupt definitions
#include "timer2.h" // ISR vector slot — see SDCC ISR-prototype-in-main rule

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

    // Timer 2 drives the matrix scan + LED PWM animation in a single
    // alternating ISR (1 matrix scan : 21 LED ticks), mirroring stock fw.
    // INT4 is not used — stock only uses it for sleep wake, which we
    // don't implement.
    timer2_init();

    EA = 1; // enable interrupts
}

void main()
{
    init();

#if DEBUG == 1
    // Paint the unused stack with a sentinel so stack_task() can later
    // measure the high-water mark. Must run after init() — running anything
    // before clock_init() bricks the keyboard (boot-RC clock + half-armed
    // peripherals); see memory feedback-no-code-before-init.
    stack_paint();
#endif

    dprint_str("SMK v" TOSTRING(SMK_VERSION) "\r\n");
    dprint_str("DEVICE vId:" TOSTRING(USB_VID) " pId:" TOSTRING(USB_PID) "\n\r");

    delay_ms(1000);

    kb_init();

#ifdef RF_ENABLED
    rf_init();
#endif

    // Load user_settings from flash. If the on-flash record fails its
    // magic/length/checksum check (fresh-flashed firmware, struct shape
    // changed, etc.), seed factory defaults instead. Either way, clamp
    // any LED-side fields that ended up out of range.
    if (!settings_load()) {
        indicators_apply_defaults();
    }
    indicators_validate_settings();

    // Enable PWM and interrupt (driving matrix scan).
    indicators_start();

#ifdef RF_ENABLED
    // user_settings.rf_link is now valid - re-establish whatever link the
    // user was last on (defaulted to RF_MODE_2_4G if no saved record).
    rf_set_link((rf_mode_t)user_settings.rf_link);
    // Optimistically prime keyboard_state to match the link we just
    // commanded so the indicator shows the right colour immediately
    // (and not the default rf_link=0 / "green" + unpaired-blink window
    // while the first status poll lags by ~100 ms). Same pattern the
    // FN+Q/W/E/R handler in kb.c uses. The supervisor's first status
    // poll will downgrade these if the BK3632 disagrees.
    keyboard_state.rf_link   = user_settings.rf_link;
    keyboard_state.connected = 1;
    keyboard_state.paired    = 1;
#endif

    delay_ms(1000);

    while (1) {
        CLR_WDT();

        kb_update_switches();
        kb_update();
        matrix_task();

        // Stock-style deferred settings save: handlers (brightness, effect,
        // rf_link, …) just flip a dirty bit; the flush happens here, once
        // per main-loop iteration. Coalesces fast successions of changes
        // into one sector erase + program run, which keeps the ~5 ms
        // erase-induced CPU stall from firing on every keypress.
        settings_task();

#if DEBUG == 1
        stack_task();
        console_task();
#endif
    }
}
