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
#include "sleep.h"
#include "power.h" // int4_isr ISR vector slot — same SDCC ISR-prototype-in-main rule

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

    // Zero the LED framebuffers before EA=1 starts the Timer-2 scan ISR — cold
    // (USB plug-in) boots don't guarantee cleared xdata, and the scan would
    // otherwise stream that garbage to the LEDs until the first render.
    indicators_init();

    // Timer 2 drives the matrix scan + LED PWM animation in a single
    // alternating ISR (1 matrix scan : 21 LED ticks).
    // INT4 is the wake-from-Power-Down source for the sleep feature (armed by
    // the board's user_sleep_prepare(); its ISR lives in power.c) — otherwise
    // it stays disabled.
    timer2_init();

    EA = 1;
}

void main()
{
    init();

#if DEBUG == 1
    // Paint the unused stack with a sentinel so stack_task() can later
    // measure the high-water mark. Must run after init() — running anything
    // before clock_init() bricks the keyboard (boot-RC clock + half-armed
    // peripherals).
    stack_paint();
#endif

    dprintf("SMK v" TOSTRING(SMK_VERSION) "\r\n");
    dprintf("KB " KEYBOARD_NAME " / " LAYOUT_NAME "\r\n");
    dprintf("DEVICE vId:" TOSTRING(USB_VID) " pId:" TOSTRING(USB_PID) "\n\r");

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
#if DEBUG == 1
    settings_dump(); // report the settings the keyboard came up with
#endif

    // Hold the backlight dark until USB has finished enumerating. The boot blip
    // is the LED scan being starved by enumeration control-transfer traffic while
    // the panel is lit, over-brightening whatever row is mid-scan. The framebuffer
    // was zeroed in init() and nothing renders until indicators_start(), so the
    // scan runs dark here; delay_ms is a busy-wait (interrupts stay on) so the
    // scan and USB ISRs keep running.
    //
    //   - USB present: usb_enum_seen latches on the first SETUP; we then wait for
    //     usb_enum_active_ticks to hit 0 (no SETUP for ~500 ms = enumeration and
    //     the HID-driver attach are done), and light up right after.
    //   - Battery / USB-power-only: no SETUP ever, so light up after a short
    //     detect window.
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

    // Cache whether this board supports Power-Down sleep (no-op when the sleep
    // feature is compiled out). Must run after the board's GPIO/RF are up.
    sleep_init();

    while (1) {
        CLR_WDT();

        kb_update_switches();
        kb_update();
        matrix_task();

        // Foreground LED render: regenerate the effect framebuffer here, out of
        // the Timer-2 scan ISR (which only streams the framebuffer to the PWM).
        // The animation phase is clocked in the ISR, so this free-runs at the
        // loop rate without affecting animation speed. No-op on LED-less boards.
        //
        indicators_render();

        // Deferred settings save: handlers (brightness, effect, rf_link, …)
        // just flip a dirty bit; the flush happens here, once
        // per main-loop iteration. Coalesces fast successions of changes
        // into one sector erase + program run, which keeps the ~5 ms
        // erase-induced CPU stall from firing on every keypress.
        settings_task();

        // Inactivity sleep: if the timeout elapsed, this drops the MCU into
        // Power-Down (LEDs off, RF asleep) and blocks until a keypress (INT4)
        // or USB event wakes it. No-op until then, and when SLEEP_ENABLE is off.
        sleep_task();

#if DEBUG == 1
        stack_task();
        console_task();
#endif
    }
}
