#include "sh68f881.h"
#include "clock.h"
#include "watchdog.h"
#include "interrupts.h"
#include "isp.h"
#include "usb.h"
#include "diag.h"
#include "console.h"
#include "debug.h"
#include "delay.h"
#include <stdint.h>

#define ISP_ESCAPE_MS 10000u

// The console is the only debug sink on this board; the SH68F90 selects between sinks
// in its uart.c, which this target does not build.
void debug_putc(char c)
{
    console_putc((unsigned char)c);
}

// The host's keyboard LED output report still arrives; nothing here drives indicators.
void keyboard_set_led_state(uint8_t led_state)
{
    (void)led_state;
}

// Bring-up image: enumerate, dump the information block to the debug console, and
// nothing else. No matrix, no LEDs.
//
// The USB block lives on SFR page 1, so page 1 stays selected for the whole run and
// the only code that leaves it is diag's info-block read, which does so with
// interrupts off. That keeps the paging out of the USB driver entirely.
void main(void)
{
    EA = 0;

    clock_init();
    sfr_page_1();

    usb_init();
    EA = 1;

    dprintf("SMK hykker bring-up\r\n");

    // Never leave the board without a way back: if the host has not configured us
    // within ISP_ESCAPE_MS, hand it to the bootloader instead of spinning here.
    uint16_t unconfigured_ms = 0;

    while (1) {
        watchdog_kick();
        diag_task();
        console_task();

        delay_ms(1);
        if (usb_is_configured()) {
            unconfigured_ms = 0;
        } else if (++unconfigured_ms > ISP_ESCAPE_MS) {
            isp_jump();
        }
    }
}
