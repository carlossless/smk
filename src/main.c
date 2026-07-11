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

    indicators_init();

    timer2_init();

    EA = 1;
}

void main()
{
    init();

#if DEBUG == 1
    stack_paint();
#endif

    dprintf("SMK v" TOSTRING(SMK_VERSION) "\r\n");
    dprintf("KB " KEYBOARD_NAME " / " LAYOUT_NAME "\r\n");
    dprintf("DEVICE vId:" TOSTRING(USB_VID) " pId:" TOSTRING(USB_PID) "\n\r");

    kb_init();

#ifdef RF_ENABLED
    rf_init();
#endif

    if (!settings_load()) {
        indicators_apply_defaults();
    }
    indicators_validate_settings();
#if DEBUG == 1
    settings_dump();
#endif

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
    rf_set_link((rf_mode_t)user_settings.rf_link);
    keyboard_state.rf_link   = user_settings.rf_link;
    keyboard_state.connected = 1;
    keyboard_state.paired    = 1;
#endif

    sleep_init();

    while (1) {
        CLR_WDT();

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
