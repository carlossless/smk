// A USB device that enumerates and does nothing else: one vendor-class interface, no
// endpoints beyond the control pipe, no strings. It exists to show that usbhw.c is usable on
// its own -- the only thing it wants from outside is usb_irq_dispatch(), supplied here.
//
// Everything that makes a keyboard a keyboard, and the HID reports with it, is in src/smk and
// none of it is linked.

#include "clock.h"
#include "ldo.h"
#include "peripherals.h"
#include "usb.h"
#include "usbhw.h"
#include "watchdog.h"
#include <stdint.h>

void usb_interrupt_handler(void) __interrupt(USB_VECTOR);

#define EP0_PACKET 8u

#define REQ_GET_DESCRIPTOR    0x06u
#define REQ_SET_ADDRESS       0x05u
#define REQ_SET_CONFIGURATION 0x09u

#define DESC_DEVICE 0x01u
#define DESC_CONFIG 0x02u

// clang-format off
static const __code uint8_t device_desc[] = {
    18, DESC_DEVICE,
    0x00, 0x02,           // USB 2.00
    0xFF, 0x00, 0x00,     // vendor class, no subclass or protocol
    EP0_PACKET,
    0x8A, 0x25,           // idVendor  0x258a
    0xAD, 0xDE,           // idProduct 0xdead
    0x00, 0x01,           // bcdDevice 1.00
    0, 0, 0,              // no strings
    1,                    // one configuration
};

static const __code uint8_t config_desc[] = {
    9, DESC_CONFIG,
    18, 0,                // wTotalLength
    1, 1,                 // one interface, configuration value 1
    0,                    // no string
    0x80,                 // bus powered, no remote wakeup
    50,                   // 100 mA

    9, 0x04,              // interface 0
    0, 0, 0,              // alternate 0, no endpoints
    0xFF, 0x00, 0x00,     // vendor class
    0,                    // no string
};
// clang-format on

static const __code uint8_t *ep0_in_src;
static uint8_t               ep0_in_left;
static uint8_t               pending_address;

static void ep0_in_step(void)
{
    uint8_t n = ep0_in_left > EP0_PACKET ? EP0_PACKET : ep0_in_left;

    for (uint8_t i = 0; i < n; i++) {
        EP0_IN_BUF[i] = ep0_in_src[i];
    }

    ep0_in_src += n;
    ep0_in_left -= n;

    SET_EP0_CNT(n);
}

static void ep0_in_start(const __code uint8_t *src, uint8_t len, uint8_t requested)
{
    ep0_in_src  = src;
    ep0_in_left = requested < len ? requested : len;

    ep0_in_step();
    SET_EP0_IN_RDY;
}

static void ep0_status_in(void)
{
    ep0_in_left = 0;
    CLEAR_EP0_CNT;
    SET_EP0_IN_RDY;
}

static void ep0_setup(void)
{
    uint8_t request   = EP0_OUT_BUF[1];
    uint8_t desc_type = EP0_OUT_BUF[3];
    uint8_t requested = EP0_OUT_BUF[6];

    switch (request) {
        case REQ_GET_DESCRIPTOR:
            if (desc_type == DESC_DEVICE) {
                ep0_in_start(device_desc, sizeof(device_desc), requested);
            } else if (desc_type == DESC_CONFIG) {
                ep0_in_start(config_desc, sizeof(config_desc), requested);
            } else {
                SET_EP0_IN_STALL;
            }
            break;

        case REQ_SET_ADDRESS:
            pending_address = EP0_OUT_BUF[2];
            ep0_status_in();
            break;

        case REQ_SET_CONFIGURATION:
            ep0_status_in();
            break;

        default:
            SET_EP0_IN_STALL;
            break;
    }
}

static void ep0_in_complete(void)
{
    if (ep0_in_left != 0) {
        ep0_in_step();
        SET_EP0_IN_RDY;
        SET_EP0_OUT_RDY;
        return;
    }

    // the address only takes effect once the host has seen the status stage
    if (pending_address != 0) {
        USBADDR         = pending_address;
        pending_address = 0;
    }

    SET_EP0_IN_STALL;
    SET_EP0_OUT_RDY;
}

void usb_irq_dispatch(void)
{
    uint8_t if1 = USBIF1;
    uint8_t if2 = USBIF2;

    if (if1 & _SOFIF) {
        USBIF1 &= ~_SOFIF;
    }

    if (if1 & _SETUPIF) {
        USBIF1 &= ~_SETUPIF;
        USBIF1 &= ~(_OVERIF | _OW);
        ep0_setup();
    }

    if (if1 & (_USBRSTIF | _SUSPIF | _RESMIF | _PUPIF)) {
        USBIF1 &= ~(_USBRSTIF | _SUSPIF | _RESMIF | _PUPIF);
    }

    if (if2 & _IEP0IF) {
        USBIF2 &= ~_IEP0IF;
        ep0_in_complete();
    }

    if (if2 & _OEP0IF) {
        USBIF2 &= ~_OEP0IF;
        CLEAR_EP0_CNT;
        SET_EP0_IN_RDY;
    }
}

void main(void)
{
    ldo_init();
    clock_init();
    peripherals_init();

    usb_hw_init();
    EA = 1;

    for (;;) {
        watchdog_kick();
    }
}
