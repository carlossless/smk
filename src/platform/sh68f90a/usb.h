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
// usb_init). Used by the RF-sleep teardown so the device is marked
// un-configured while the USB PHY is powered down.
void    usb_deinit();
void    usb_send_report(__xdata report_keyboard_t *report);
void    usb_send_nkro(__xdata report_nkro_t *report);
void    usb_send_extra(__xdata report_extra_t *report);
uint8_t usb_device_state_get_protocol();

// Counts down (in 1 ms USB SOF ticks) since the last SETUP packet. The USB ISR
// reloads it on every SETUP and decrements it on every SOF, so it stays > 0 for
// as long as the host is actively running control transfers (enumeration + HID
// driver attach) and reaches 0 once the bus has been quiet for the reload
// window. main() holds the backlight dark until this first hits 0, so the
// one-time enumeration traffic that disrupts the LED scan lands while nothing is
// lit. Zero on battery (no SOF/SETUP ever), so wireless boots light immediately.
extern __xdata uint16_t usb_enum_active_ticks;

// Latches true on the first SETUP packet (cleared in usb_init). Lets main()
// distinguish "USB is enumerating, wait for it to finish" from "no USB host
// here (battery / power-only), light up after a short detect window".
extern __xdata bool usb_enum_seen;

// Host-controlled remote-wakeup enable (USB SET/CLEAR_FEATURE(DEVICE_REMOTE_WAKEUP)).
// The RF sleep wake path only signals USB resume (USBCON.WKUP) when the host has
// armed remote wakeup.
extern __bit usb_remote_wakeup;

// True while the host has the bus suspended (set on SUSPIF when configured,
// cleared on the next SOF / bus reset). The sleep feature uses this as the
// USB-mode sleep trigger — see src/smk/sleep.c.
extern __bit usb_suspended;

#if DEBUG == 1
bool usb_is_configured();
#endif // DEBUG

// __using would reserve a register bank from DSEG, which we can't afford
// right now — DSEG is already at max usage. Falls back to default (bank 0
// with push/pop of R0-R7 on entry/exit). If we ever free 8 bytes of DSEG,
// add __using(2) here (must differ from PWM/Timer 2 which use bank 1).
void usb_interrupt_handler() __interrupt(_INT_USB);
