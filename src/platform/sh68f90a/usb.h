#pragma once

#include "sh68f90a.h"
#include "report.h"
#include <stdint.h>

enum {
    USB_PROTOCOL_BOOT   = 0,
    USB_PROTOCOL_REPORT = 1,
};

void usb_init();
// Reset the USB device-state machine + disable the module (counterpart to
// usb_init). Marks the device un-configured while the PHY is powered down.
void    usb_deinit();
void    usb_send_report(__xdata report_keyboard_t *report);
void    usb_send_nkro(__xdata report_nkro_t *report);
void    usb_send_extra(__xdata report_extra_t *report);
uint8_t usb_device_state_get_protocol();

// Counts down in 1 ms SOF ticks since the last SETUP: reloaded on SETUP,
// decremented on SOF, so it stays > 0 while the host runs control transfers
// (enumeration + HID attach) and reaches 0 once the bus goes quiet. main() holds
// the backlight dark until this first hits 0, so the enumeration traffic that
// disrupts the LED scan lands while nothing is lit. Zero on battery (no SOF).
extern __xdata uint16_t usb_enum_active_ticks;

// Latches on the first SETUP (cleared in usb_init). Lets main() tell "USB is
// enumerating, wait" from "no USB host, light up after a short window".
extern __xdata bool usb_enum_seen;

// Host-controlled remote-wakeup enable. The RF sleep wake path only signals USB
// resume (USBCON.WKUP) when the host has armed remote wakeup.
extern __bit usb_remote_wakeup;

// True while the host has the bus suspended (set on SUSPIF when configured,
// cleared on the next SOF / bus reset). The USB-mode sleep trigger.
extern __bit usb_suspended;

#if DEBUG == 1
bool usb_is_configured();
#endif // DEBUG

// No __using: reserving a register bank costs DSEG we can't afford, so this
// falls back to bank 0 with push/pop of R0-R7. If 8 bytes of DSEG ever free up,
// add __using(2) here (must differ from PWM/Timer 2's bank 1).
void usb_interrupt_handler() __interrupt(_INT_USB);
