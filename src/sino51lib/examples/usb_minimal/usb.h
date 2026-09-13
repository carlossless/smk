#pragma once

// usbhw.c calls out to exactly one function, and this is it. The library brings the SIE up
// and moves bytes; deciding what the device *is* belongs to whoever is using it, which is why
// this header lives beside the example and not in the library.
//
// The firmware in src/smk has its own, much larger, version of this file.

void usb_irq_dispatch(void);
