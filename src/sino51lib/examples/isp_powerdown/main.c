// The two ways out of the running application: down into Power-Down, and out into the ISP
// bootloader.
//
// DO NOT FLASH THIS ONE WITHOUT READING THE PART'S SLEEP NOTES. power_enter_powerdown() stops
// the clock tree and only a wake source brings it back: on the SH68F89 a USB plug, reset or
// resume is enough, but on the SH68F881 nothing but an external interrupt, Timer3 or the base
// timer will do, and this example arms none of those. extint_wake_arm() is a no-op on both --
// which pin a keypress reaches is the board's to say, not the library's.
//
// isp_jump() does not return: it hands control to the bootloader, which is how sinowisp
// reflashes a running board.

#include "clock.h"
#include "delay.h"
#include "extint.h"
#include "isp.h"
#include "ldo.h"
#include "peripherals.h"
#include "power.h"
#include "usb.h"
#include "watchdog.h"
#include <stdint.h>

#define AWAKE_MS 2000u
#define ROUNDS   3u

// what power.c calls on the way down and back up; an application with a USB device puts its
// own bring-up here
__bit usb_suspended;

void usb_init(void) {}
void usb_deinit(void) {}

void main(void)
{
    ldo_init();
    clock_init();
    peripherals_init();

    for (uint8_t round = 0; round < ROUNDS; round++) {
        watchdog_kick();
        delay_ms(AWAKE_MS);

        extint_wake_clear();
        extint_wake_arm();

        // keeps the device attached across the sleep, so a host-side resume can end it
        power_enter_powerdown(POWERDOWN_KEEP_USB_ALIVE);

        extint_wake_disable();
    }

    isp_jump();
}
