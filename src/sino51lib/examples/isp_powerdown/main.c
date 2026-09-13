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

        power_enter_powerdown(POWERDOWN_KEEP_USB_ALIVE);

        extint_wake_disable();
    }

    isp_jump();
}
