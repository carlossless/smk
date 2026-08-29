#include "clock.h"
#include "ldo.h"
#include "watchdog.h"
#include "interrupts.h"
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
#include "tick.h"
#include "sleep.h"
#ifdef DEBUG_SINK_UART
#    include "uart.h"
#endif
#ifdef RF_ENABLED
#    include "rf_controller.h"
#endif

// Not static: tests/sim.py breakpoints on the `LCALL _init` in main() to run
// assertions against a fully initialised device.
void init(void)
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

    // Zero the LED framebuffers before EA=1 lets the tick ISR start streaming
    // them: a cold boot doesn't guarantee cleared xdata, so the first frames
    // would go out as garbage.
    indicators_init();

    tick_init();

    EA = 1;
}

static void restore_settings(void)
{
    if (!settings_load()) {
        indicators_apply_defaults();
    }
    indicators_validate_settings();
}

#ifdef RF_ENABLED
static void restore_rf_link(void)
{
    rf_set_link((rf_mode_t)user_settings.rf_link);

    keyboard_state.rf_link   = user_settings.rf_link;
    keyboard_state.connected = 1;
    keyboard_state.paired    = 1;
}
#endif

void main(void)
{
    init();

#if DEBUG == 1
    // Paint the unused stack with a sentinel for stack_task()'s high-water
    // measurement. Must run after init() - anything before clock_init() bricks
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

    restore_settings();
#if DEBUG == 1
    settings_dump();
#endif

    usb_wait_for_enumeration();
    indicators_start();

#ifdef RF_ENABLED
    restore_rf_link();
#endif

    sleep_init(); // needs the board's GPIO and RF up

    while (1) {
        watchdog_kick();

        kb_update_switches();
        kb_update();
        matrix_task();

        indicators_render();

        settings_task();
        sleep_task();

#if DEBUG == 1
        stack_task();
        console_task();
#endif
    }
}
