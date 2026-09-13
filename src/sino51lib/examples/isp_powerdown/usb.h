#pragma once

// power.c parks and restores the USB device around the sleep, which it can only do by calling
// the application: bringing the device back up after the clock tree restarts is not something
// the library can know how to do. The firmware in src/smk has the real ones; here they are
// stubs, so this example sleeps and wakes without a device attached.

extern __bit usb_suspended;

void usb_init(void);
void usb_deinit(void);
