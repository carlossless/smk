#include "usb.h"
#include "watchdog.h"
#include "isp.h"
#include "../../smk/usbdef.h"
#include "usbregs.h"
#include "../../smk/debug.h"
#include "../../smk/utils.h"
#include "../../smk/usbhidreport.h"
#include "../../smk/keyboard.h"
#include <stdint.h>
#include <string.h>

#ifndef USB_VID
#    define USB_VID 0xdead
#endif

#ifndef USB_PID
#    define USB_PID 0xbeef
#endif

#define BCDHID 0x0111 // HID Class Spec Version

enum usb_string_index {
    USB_STRING_LANGUAGE_ID   = 0,
    USB_STRING_MANUFACTURER  = 1,
    USB_STRING_PRODUCT       = 2,
    USB_STRING_SERIAL_NUMBER = 3,
};

enum report_id {
    REPORT_ID_ACPI     = 1,
    REPORT_ID_CONSUMER = 2,
    REPORT_ID_ISP      = 5,
    REPORT_ID_NKRO     = 6,
};

typedef enum {
    USB_EP0_STATE_DEFAULT     = 0x00,
    USB_EP0_STATE_IN_DATA     = 0x01,
    USB_EP0_STATE_RECV_STATUS = 0x02,
    USB_EP0_STATE_LED         = 0x04,
    USB_EP0_STATE_ISP         = 0x05,
} usb_ep0_state_t;

const uint8_t hid_report_desc_keyboard[] = {
    // clang-format off
    HID_RI_USAGE_PAGE(8, 0x01),     // Generic Desktop Controls
    HID_RI_USAGE(8, 0x06),          // System Control
    HID_RI_COLLECTION(8, 0x01),     // Application
        // Modifiers (8 bits)
        HID_RI_USAGE_PAGE(8, 0x07),     // Keyboard/Keypad
        HID_RI_USAGE_MINIMUM(8, 0xe0),  // Keyboard Left Control
        HID_RI_USAGE_MAXIMUM(8, 0xe7),  // Keyboard Right Gui
        HID_RI_LOGICAL_MINIMUM(8, 0x00),
        HID_RI_LOGICAL_MAXIMUM(8, 0x01),
        HID_RI_REPORT_SIZE(8, 0x01),
        HID_RI_REPORT_COUNT(8, 0x08),
        HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),

        // Reserved (1 byte)
        HID_RI_REPORT_SIZE(8, 0x08),
        HID_RI_REPORT_COUNT(8, 0x01),
        HID_RI_INPUT(8, HID_IOF_CONSTANT),

        // Keycodes (6 bytes)
        HID_RI_USAGE_PAGE(8, 0x07),    // Keyboard/Keypad
        HID_RI_USAGE_MINIMUM(8, 0x00),
        HID_RI_USAGE_MAXIMUM(8, 0xFF),
        HID_RI_LOGICAL_MINIMUM(8, 0x00),
        HID_RI_LOGICAL_MAXIMUM(16, 0x00FF),
        HID_RI_REPORT_SIZE(8, 0x08),
        HID_RI_REPORT_COUNT(8, 0x06),
        HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_ARRAY | HID_IOF_ABSOLUTE),

        // Status LEDs (5 bits)
        HID_RI_USAGE_PAGE(8, 0x08),    // LED
        HID_RI_USAGE_MINIMUM(8, 0x01), // Num Lock
        HID_RI_USAGE_MAXIMUM(8, 0x05), // Kana
        HID_RI_LOGICAL_MINIMUM(8, 0x00),
        HID_RI_LOGICAL_MAXIMUM(8, 0x01),
        HID_RI_REPORT_SIZE(8, 0x01),
        HID_RI_REPORT_COUNT(8, 0x05),
        HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),

        // LED padding (3 bits)
        HID_RI_REPORT_SIZE(8, 0x03),
        HID_RI_REPORT_COUNT(8, 0x01),
        HID_RI_OUTPUT(8, HID_IOF_CONSTANT),
    HID_RI_END_COLLECTION(0),
    // clang-format on
};

const uint8_t hid_report_desc_extra[] = {
    // clang-format off
    HID_RI_USAGE_PAGE(8, 0x01),           // Generic Desktop
    HID_RI_USAGE(8, 0x80),                // System Control
    HID_RI_COLLECTION(8, 0x01),           // Application
        HID_RI_REPORT_ID(8, REPORT_ID_ACPI),        // ACPI
        HID_RI_USAGE_MINIMUM(8, 0x81),
        HID_RI_USAGE_MAXIMUM(8, 0x83),
        HID_RI_LOGICAL_MINIMUM(8, 0x00),
        HID_RI_LOGICAL_MAXIMUM(8, 0x01),
        HID_RI_REPORT_SIZE(8, 1),
        HID_RI_REPORT_COUNT(8, 3),
        HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE),
        HID_RI_REPORT_COUNT(8, 5),
        HID_RI_INPUT(8, HID_IOF_CONSTANT),
    HID_RI_END_COLLECTION(0),

    HID_RI_USAGE_PAGE(8, 0x0C),           // Consumer
    HID_RI_USAGE(8, 0x01),                // Consumer Control
    HID_RI_COLLECTION(8, 0x01),           // Application
        HID_RI_REPORT_ID(8, REPORT_ID_CONSUMER),
        HID_RI_USAGE_MINIMUM(8, 0x00),
        HID_RI_USAGE_MAXIMUM(16, 0x023c),
        HID_RI_LOGICAL_MINIMUM(8, 0x00),
        HID_RI_LOGICAL_MAXIMUM(16, 0x023c),
        HID_RI_REPORT_SIZE(8, 16),
        HID_RI_REPORT_COUNT(8, 1),
        HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
    HID_RI_END_COLLECTION(0),

    HID_RI_USAGE_PAGE(16, 0xff00),        // Vendor
    HID_RI_USAGE(8, 0x01),                // Vendor
    HID_RI_COLLECTION(8, 0x01),           // Application
        HID_RI_REPORT_ID(8, REPORT_ID_ISP),        // ISP
        HID_RI_USAGE_MINIMUM(8, 0x01),
        HID_RI_USAGE_MAXIMUM(8, 0x02),
        HID_RI_LOGICAL_MINIMUM(8, 0x00),
        HID_RI_LOGICAL_MAXIMUM(16, 0x00ff),
        HID_RI_REPORT_SIZE(8, 8),
        HID_RI_REPORT_COUNT(8, 5),
        HID_RI_FEATURE(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
    HID_RI_END_COLLECTION(0),

    HID_RI_USAGE_PAGE(8, 0x01),           // Generic Desktop
    HID_RI_USAGE(8, 0x06),                // Keyboard
    HID_RI_COLLECTION(8, 0x01),           // Application
        HID_RI_REPORT_ID(8, REPORT_ID_NKRO),
        // Modifiers (8 bits)
        HID_RI_USAGE_PAGE(8, 0x07),       // Keyboard/Keypad
        HID_RI_USAGE_MINIMUM(8, 0x04),
        HID_RI_USAGE_MAXIMUM(8, 0x70),
        HID_RI_LOGICAL_MINIMUM(8, 0x00),
        HID_RI_LOGICAL_MAXIMUM(8, 0x01),
        HID_RI_REPORT_SIZE(8, 1),
        HID_RI_REPORT_COUNT(8, 120),
        HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
    HID_RI_END_COLLECTION(0),
    // clang-format on
};

usb_desc_device_c usb_desc_device = {
    .bLength            = sizeof(struct usb_desc_device),
    .bDescriptorType    = USB_DESC_DEVICE,
    .bcdUSB             = 0x0110,
    .bDeviceClass       = USB_DEV_CLASS_PER_INTERFACE,
    .bDeviceSubClass    = USB_DEV_SUBCLASS_PER_INTERFACE,
    .bDeviceProtocol    = USB_DEV_PROTOCOL_PER_INTERFACE,
    .bMaxPacketSize0    = 8,
    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0000,
    .iManufacturer      = USB_STRING_MANUFACTURER,
    .iProduct           = USB_STRING_PRODUCT,
    .iSerialNumber      = USB_STRING_SERIAL_NUMBER,
    .bNumConfigurations = 1,
};

usb_desc_interface_c usb_desc_interface_main = {
    .bLength            = sizeof(struct usb_desc_interface),
    .bDescriptorType    = USB_DESC_INTERFACE,
    .bInterfaceNumber   = 0,
    .bAlternateSetting  = 0,
    .bNumEndpoints      = 1,
    .bInterfaceClass    = USB_IFACE_CLASS_HID,
    .bInterfaceSubClass = USB_IFACE_SUBCLASS_HID_BOOT,
    .bInterfaceProtocol = USB_IFACE_PROTOCOL_REPORT,
    .iInterface         = 0,
};

usb_desc_hid_c usb_desc_hid_main = {
    .bLength            = sizeof(struct usb_desc_hid),
    .bDescriptorType    = USB_DESC_CLASS_HID,
    .bcdHID             = BCDHID,
    .bCountryCode       = 0,
    .bNumDescriptors    = 1,
    .bDescriptorType1   = USB_DESC_CLASS_REPORT,
    .wDescriptorLength1 = sizeof(hid_report_desc_keyboard),
};

usb_desc_endpoint_c usb_desc_endpoint_main = {
    .bLength          = sizeof(struct usb_desc_endpoint),
    .bDescriptorType  = USB_DESC_ENDPOINT,
    .bEndpointAddress = 1 | USB_DIR_IN,
    .bmAttributes     = USB_XFER_INTERRUPT,
    .wMaxPacketSize   = 8, // 8 bytes
    .bInterval        = 1, // 1ms
};

usb_desc_interface_c usb_desc_interface_extra = {
    .bLength            = sizeof(struct usb_desc_interface),
    .bDescriptorType    = USB_DESC_INTERFACE,
    .bInterfaceNumber   = 1,
    .bAlternateSetting  = 0,
    .bNumEndpoints      = 1,
    .bInterfaceClass    = USB_IFACE_CLASS_HID,
    .bInterfaceSubClass = USB_IFACE_SUBCLASS_NONE,
    .bInterfaceProtocol = USB_IFACE_PROTOCOL_BOOT,
    .iInterface         = 0,
};

usb_desc_hid_c usb_desc_hid_extra = {
    .bLength            = sizeof(struct usb_desc_hid),
    .bDescriptorType    = USB_DESC_CLASS_HID,
    .bcdHID             = BCDHID,
    .bCountryCode       = 0,
    .bNumDescriptors    = 1,
    .bDescriptorType1   = USB_DESC_CLASS_REPORT,
    .wDescriptorLength1 = sizeof(hid_report_desc_extra),
};

usb_desc_endpoint_c usb_desc_endpoint_extra = {
    .bLength          = sizeof(struct usb_desc_endpoint),
    .bDescriptorType  = USB_DESC_ENDPOINT,
    .bEndpointAddress = 2 | USB_DIR_IN,
    .bmAttributes     = USB_XFER_INTERRUPT,
    .wMaxPacketSize   = 16, // 16 bytes
    .bInterval        = 1,  // 1ms
};

usb_configuration_c usb_config = {
    {
        .bLength             = sizeof(struct usb_desc_configuration),
        .bDescriptorType     = USB_DESC_CONFIGURATION,
        .bNumInterfaces      = 2,
        .bConfigurationValue = 1,
        .iConfiguration      = 0,
        .bmAttributes        = USB_ATTR_RESERVED_1 | USB_ATTR_REMOTE_WAKEUP,
        .bMaxPower           = 250, // 500mA
    },
    {
        {.interface = &usb_desc_interface_main},
        {.hid = &usb_desc_hid_main},
        {.endpoint = &usb_desc_endpoint_main},
        {.interface = &usb_desc_interface_extra},
        {.hid = &usb_desc_hid_extra},
        {.endpoint = &usb_desc_endpoint_extra},
        {0},
    },
};

usb_configuration_set_c usb_configs[] = {
    &usb_config,
};

static usb_desc_langid_c usb_langid = {
    .bLength         = sizeof(struct usb_desc_langid) + sizeof(uint16_t) * 1,
    .bDescriptorType = USB_DESC_STRING,
    .wLANGID         = {/* English (United States) */ 0x0409},
};

usb_ascii_string_c usb_strings[] = {
    [USB_STRING_MANUFACTURER - 1]  = "contact@carlossless.io",
    [USB_STRING_PRODUCT - 1]       = "SMK Keyboard",
    [USB_STRING_SERIAL_NUMBER - 1] = "0001",
};

usb_descriptor_set_c usb_descriptor_set = {
    .device       = &usb_desc_device,
    .config_count = ARRAYSIZE(usb_configs),
    .configs      = usb_configs,
    .string_count = ARRAYSIZE(usb_strings),
    .strings      = usb_strings,
};

// interrupt handlers
static void usb_setup_irq();
static void usb_ep0_out_irq();
static void usb_ep0_in_irq();

// buffer utils
static void setup_ep0_in_xfer(uint8_t *src, uint16_t len);
static void step_ep0_in_xfer();
static void set_ep0_in_buffer(uint8_t *src, uint8_t len);
static void get_ep0_out_buffer(uint8_t *dest);
static void set_ep1_in_buffer(uint8_t *src, uint8_t len);
static void get_ep1_out_buffer(uint8_t *dest);

// request handlers
static void usb_clear_remote_wakeup_handler(__xdata struct usb_req_setup *req);
static void usb_clear_endpoint_halt_handler(__xdata struct usb_req_setup *req);
static void usb_set_remote_wakeup_handler(__xdata struct usb_req_setup *req);
static void usb_set_endpoint_halt_handler(__xdata struct usb_req_setup *req);
static void usb_set_address_handler(__xdata struct usb_req_setup *req);
static void usb_set_configuration_handler(__xdata struct usb_req_setup *req);
static void usb_set_interface_handler(__xdata struct usb_req_setup *req);
static void usb_set_descriptor_handler();
static void usb_get_device_status_handler(__xdata struct usb_req_setup *req);
static void usb_get_interface_status_handler(__xdata struct usb_req_setup *req);
static void usb_get_endpoint_status_handler(__xdata struct usb_req_setup *req);
static void usb_get_descriptor_handler(__xdata struct usb_req_setup *req);
static void usb_get_configuration_handler();
static void usb_get_interface_handler();
static void usb_hid_get_report_handler();
static void usb_hid_set_report_handler(__xdata struct usb_req_setup *req);
static void usb_hid_set_idle_handler(__xdata struct usb_req_setup *req);
static void usb_hid_get_idle_handler();
static void usb_hid_set_protocol_handler(__xdata struct usb_req_setup *req);
static void usb_hid_get_protocol_handler(__xdata struct usb_req_setup *req);

/**
 * 0.5KB of general purpose scratch RAM.
 */
__xdata uint8_t scratch[512];

uint16_t ep0_xfer_bytes_left;
uint8_t *ep0_xfer_src;

// usb state
usb_device_state_t __xdata usb_device_state;
uint8_t __xdata            received_usb_addr;
uint8_t __xdata            active_configuration;
uint8_t __xdata            interface0_protocol;
uint8_t __xdata            interface1_protocol;
__bit                      usb_remote_wakeup;
uint8_t __xdata            idle_time;
usb_ep0_state_t __xdata    usb_ep0_state;

void usb_init()
{
    usb_device_state     = USB_DEVICE_STATE_DEFAULT;
    received_usb_addr    = 0;
    active_configuration = 0;
    interface0_protocol  = 0;
    interface1_protocol  = 0;
    usb_remote_wakeup    = 0;
    usb_ep0_state        = USB_EP0_STATE_DEFAULT;
    idle_time            = 0;

    ep0_xfer_bytes_left = 0;
    ep0_xfer_src        = 0;

    USBADDR = 0;
    USBIE1  = (_OVERIE | _SETUPIE | _SOFIA | _RESMIE | _SUSPIE | _PBRSTIE);
    USBIE2  = (_OEP0IE | _IEP0IE);
    USBCON  = (_ENUSB | _SW1CON);
    IEN1 |= _EUSB;
}

void usb_send_report(report_keyboard_t *report)
{
    set_ep1_in_buffer(report->raw, KEYBOARD_REPORT_SIZE);

    SET_EP1_CNT(8);
    SET_EP1_IN_RDY;
}

static void usb_setup_irq()
{
    usb_req_setup_x req;

    EA = 0;
    get_ep0_out_buffer((uint8_t *)&req);
    EA = 1;

    uint8_t type    = req.bmRequestType;
    uint8_t request = req.bRequest;

    switch (type) {
        case (USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE):
            switch (request) {
                case USB_REQ_CLEAR_FEATURE:
                    usb_clear_remote_wakeup_handler(&req);
                    break;

                case USB_REQ_SET_FEATURE:
                    usb_set_remote_wakeup_handler(&req);
                    break;

                case USB_REQ_SET_ADDRESS:
                    usb_set_address_handler(&req);
                    break;

                case USB_REQ_SET_DESCRIPTOR:
                    usb_set_descriptor_handler();
                    break;

                case USB_REQ_SET_CONFIGURATION:
                    usb_set_configuration_handler(&req);
                    break;

                default:
                    STALL_EP0();
                    return;
            }

            break;

        case (USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE):
            switch (request) {
                case USB_REQ_GET_STATUS:
                    usb_get_device_status_handler(&req);
                    break;

                case USB_REQ_GET_DESCRIPTOR:
                    usb_get_descriptor_handler(&req);
                    break;

                case USB_REQ_GET_CONFIGURATION:
                    usb_get_configuration_handler();
                    break;

                default:
                    STALL_EP0();
                    return;
            }

            break;

        case (USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_IFACE):
            switch (request) {
                case USB_REQ_SET_INTERFACE:
                    usb_set_interface_handler(&req);
                    break;

                default:
                    STALL_EP0();
                    return;
            }

            break;

        case (USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_IFACE):
            switch (request) {
                case USB_REQ_GET_STATUS:
                    usb_get_interface_status_handler(&req);
                    break;

                case USB_REQ_GET_DESCRIPTOR:
                    usb_get_descriptor_handler(&req);
                    break;

                case USB_REQ_GET_INTERFACE:
                    usb_get_interface_handler();
                    break;

                default:
                    STALL_EP0();
                    return;
            }

            break;

        case (USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_ENDPT):
            switch (request) {
                case USB_REQ_CLEAR_FEATURE:
                    usb_clear_endpoint_halt_handler(&req);
                    break;

                case USB_REQ_SET_FEATURE:
                    usb_set_endpoint_halt_handler(&req);
                    break;

                default:
                    STALL_EP0();
                    return;
            }

            break;

        case (USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_ENDPT):
            switch (request) {
                case USB_REQ_GET_STATUS:
                    usb_get_endpoint_status_handler(&req);
                    break;

                default:
                    STALL_EP0();
                    return;
            }

            break;

        case (USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_IFACE):
            switch (request) {
                case USB_HID_REQ_SET_REPORT:
                    usb_hid_set_report_handler(&req);
                    break;

                case USB_HID_REQ_SET_IDLE:
                    usb_hid_set_idle_handler(&req);
                    break;

                case USB_HID_REQ_SET_PROTOCOL:
                    usb_hid_set_protocol_handler(&req);
                    break;

                default:
                    STALL_EP0();
                    return;
            }

            break;

        case (USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_IFACE):
            switch (request) {
                case USB_HID_REQ_GET_REPORT:
                    usb_hid_get_report_handler();
                    break;

                case USB_HID_REQ_GET_IDLE:
                    usb_hid_get_idle_handler();
                    break;

                case USB_HID_REQ_GET_PROTOCOL:
                    usb_hid_get_protocol_handler(&req);
                    break;

                default:
                    STALL_EP0();
                    return;
            }

            break;

        default:
            STALL_EP0();
            return;
    }
}

void usb_interrupt_handler() __interrupt(_INT_USB)
{
    uint8_t temp_usbif1 = USBIF1;
    uint8_t temp_usbif2 = USBIF2;

    if (temp_usbif1 != 0x00) {
        if (temp_usbif1 & _SOFIF) {
            USBIF1 &= ~_SOFIF;
        } else {
            // why is this being cleared and why in this fashion?
            USBIF1 &= ~(_SETUPIF);      // Clear SETUPIF
            USBIF1 &= ~(_OVERIF | _OW); // SETUP OVERIF & OW

            if (temp_usbif1 & _PUPIF) {
                USBIF1 &= ~_PUPIF;
                usb_init();
            } else if (temp_usbif1 & _SETUPIF) {
                USBIF1 &= ~_SETUPIF;
                usb_setup_irq();
            } else if (temp_usbif1 & _RESMIF) { // RESMIF
                USBIF1 &= ~_RESMIF;
            } else if (temp_usbif1 & _SUSPIF) { // SUSPIF
                USBIF1 &= ~_SUSPIF;
            } else if (temp_usbif1 & _USBRSTIF) { // BUSRSTIF
                USBIF1 &= ~_USBRSTIF;

                USBCON |= _SWRST; // softreset
                _nop_();
                _nop_();
                _nop_();
                _nop_();
                _nop_();
                _nop_();
                USBCON &= ~_SWRST; // clear softreset

                usb_init();
                SET_EP0_OUT_RDY;
            }
        }
    } else if (temp_usbif2 != 0x00) {
        if (temp_usbif2 & _OEP2IF) {
            USBIF2 &= ~_OEP2IF;
            SET_EP2_OUT_RDY;
        } else if (temp_usbif2 & _OEP1IF) {
            USBIF2 &= ~_OEP1IF;
        } else if (temp_usbif2 & _OEP0IF) {
            USBIF2 &= ~_OEP0IF;
            usb_ep0_out_irq();
            SET_EP0_OUT_RDY;
        } else if (temp_usbif2 & _IEP2IF) {
            USBIF2 &= ~_IEP2IF;
        } else if (temp_usbif2 & _IEP1IF) {
            USBIF2 &= ~_IEP1IF;
        } else if (temp_usbif2 & _IEP0IF) {
            USBIF2 &= ~_IEP0IF;
            usb_ep0_in_irq();
        }
    }
}

// request handlers

static void usb_set_address_handler(__xdata struct usb_req_setup *req)
{
    usb_ep0_state = USB_EP0_STATE_DEFAULT;

    if (req->wValue != 0) {
        usb_device_state = USB_DEVICE_STATE_ADDRESSED;
    } else {
        usb_device_state = USB_DEVICE_STATE_DEFAULT;
    }

    received_usb_addr = req->wValue;

    CLEAR_EP0_CNT;
    SET_EP0_OUT_STALL;
    SET_EP0_IN_RDY;
}

static void usb_clear_remote_wakeup_handler(__xdata struct usb_req_setup *req)
{
    usb_ep0_state = USB_EP0_STATE_DEFAULT;

    if ((req->bmRequestType & USB_RECIP_MASK) == USB_RECIP_DEVICE) {
        if (req->wValue == USB_FEAT_DEVICE_REMOTE_WAKEUP) {
            usb_remote_wakeup = 0;
        }
    }

    CLEAR_EP0_CNT;
    SET_EP0_IN_RDY;
}

static void usb_set_remote_wakeup_handler(__xdata struct usb_req_setup *req)
{
    usb_ep0_state = USB_EP0_STATE_DEFAULT;

    if ((req->bmRequestType & USB_RECIP_MASK) == USB_RECIP_DEVICE) {
        if (req->wValue == USB_FEAT_DEVICE_REMOTE_WAKEUP) {
            usb_remote_wakeup = 1;
        }
    }

    CLEAR_EP0_CNT;
    SET_EP0_IN_RDY;
}

static void usb_clear_endpoint_halt_handler(__xdata struct usb_req_setup *req)
{
    usb_ep0_state = USB_EP0_STATE_DEFAULT;

    if ((req->bmRequestType & USB_RECIP_MASK) == USB_RECIP_ENDPT) {
        if (req->wValue == USB_FEAT_ENDPOINT_HALT) {
            if (req->wIndex == 0) {
                CANCEL_EP0_IN_STALL;
                CANCEL_EP0_OUT_STALL;
            } else if (req->wIndex == 0x81) {
                CANCEL_EP1_IN_STALL;
            } else if (req->wIndex == 0x82) {
                CANCEL_EP2_IN_STALL;
            } else {
                STALL_EP0();
                return;
            }
        }
    }

    CLEAR_EP0_CNT;
    SET_EP0_IN_RDY;
}

static void usb_set_endpoint_halt_handler(__xdata struct usb_req_setup *req)
{
    usb_ep0_state = USB_EP0_STATE_DEFAULT;

    if ((req->bmRequestType & USB_RECIP_MASK) == USB_RECIP_ENDPT) {
        if (req->wIndex == 0x81) {
            SET_EP1_IN_STALL;
        } else if (req->wIndex == 0x82) {
            SET_EP2_IN_STALL;
        } else {
            STALL_EP0();
        }
    } else {
        STALL_EP0();
        return;
    }

    CLEAR_EP0_CNT;
    SET_EP0_IN_RDY;
}

static void usb_set_configuration_handler(__xdata struct usb_req_setup *req)
{
    usb_ep0_state = USB_EP0_STATE_DEFAULT;

    if (USBADDR) {
        if ((req->wValue == 0) || (req->wValue == 1)) {
            active_configuration = req->wValue;

            if (active_configuration != 0) {
                usb_device_state = USB_DEVICE_STATE_CONFIGURED;
            } else {
                usb_device_state = USB_DEVICE_STATE_ADDRESSED;
            }

            CLEAR_EP0_CNT;
            SET_EP0_IN_RDY;
        } else {
            STALL_EP0();
        }
    } else {
        STALL_EP0();
    }

    CLEAR_EP0_CNT;
    SET_EP0_IN_RDY;
}

static void usb_set_interface_handler(__xdata struct usb_req_setup *req)
{
    usb_ep0_state = USB_EP0_STATE_DEFAULT;

    if (req->wValue == 0) {
        if (req->wIndex == 0) {
            CLEAR_EP0_CNT;
            SET_EP0_IN_RDY;
            SET_EP0_OUT_STALL;
        } else if (req->wIndex == 1) {
            CLEAR_EP0_CNT;
            SET_EP0_IN_RDY;
            SET_EP0_OUT_STALL;
        } else {
            STALL_EP0();
        }
    } else {
        STALL_EP0();
    }
}

static void usb_set_descriptor_handler()
{
    STALL_EP0();
}

static void usb_get_device_status_handler(__xdata struct usb_req_setup *req)
{
    usb_ep0_state = USB_EP0_STATE_RECV_STATUS;

    if ((req->bmRequestType & USB_RECIP_MASK) == USB_RECIP_DEVICE) {
        if (usb_remote_wakeup) {
            EP0_IN_BUF[0] = 0x02;
        } else {
            EP0_IN_BUF[0] = 0x00;
        }

        EP0_IN_BUF[1] = 0;

        SET_EP0_CNT(2);
        SET_EP0_IN_RDY;
    } else {
        STALL_EP0();
    }
}

static void usb_get_interface_status_handler(__xdata struct usb_req_setup *req)
{
    if ((req->bmRequestType & USB_RECIP_MASK) == USB_RECIP_IFACE) {
        if ((req->wIndex == 0) || (req->wIndex == 1)) {
            EP0_IN_BUF[0] = 0;
            EP0_IN_BUF[1] = 0;

            SET_EP0_CNT(2);
            SET_EP0_IN_RDY;
        } else {
            STALL_EP0();
        }
    } else {
        STALL_EP0();
    }
}

static void usb_get_endpoint_status_handler(__xdata struct usb_req_setup *req)
{
    if ((req->bmRequestType & USB_RECIP_MASK) == USB_RECIP_ENDPT) {
        if (req->wIndex == 0x80) {
            EP0_IN_BUF[0] = ((EP0CON & 0x02)) >> 1;
            EP0_IN_BUF[1] = 0;
        } else if (req->wIndex == 0x81) {
            EP0_IN_BUF[0] = ((EP1CON & 0x08)) >> 3;
            EP0_IN_BUF[1] = 0;
        } else if (req->wIndex == 0x82) {
            EP0_IN_BUF[0] = ((EP2CON & 0x08)) >> 3;
            EP0_IN_BUF[1] = 0;
        } else {
            STALL_EP0();
            return;
        }

        SET_EP0_CNT(2);
        SET_EP0_IN_RDY;
    } else {
        STALL_EP0();
    }
}

static void usb_get_descriptor_handler(__xdata struct usb_req_setup *req)
{
    uint8_t *__xdata addr;
    uint16_t __xdata length;

    __xdata uint8_t      *buf   = scratch;
    uint8_t               type  = req->wValue >> 8;
    uint8_t               index = req->wValue & 0xff;
    usb_descriptor_set_c *set   = &usb_descriptor_set;

#define APPEND(addr, length)       \
    do {                           \
        memcpy(buf, addr, length); \
        buf += length;             \
    } while (0)

#define APPEND_DESC(desc) APPEND(desc, (desc)->bLength)

    if (type == USB_DESC_DEVICE) {
        APPEND_DESC(set->device);

        addr   = scratch;
        length = scratch[0];
    } else if (type == USB_DESC_CONFIGURATION && index < set->config_count) {
        usb_configuration_c                   *config      = set->configs[index];
        __xdata struct usb_desc_configuration *config_desc = (__xdata struct usb_desc_configuration *)buf;
        __code const union usb_config_item    *config_item = &config->items[0];

        APPEND_DESC(&config->desc);

        do {
            APPEND_DESC(config_item->generic);
        } while ((++config_item)->generic);

        // Fix up wTotalLength so we don't need to calculate it explicitly.
        if (config_desc->wTotalLength == 0) {
            config_desc->wTotalLength = (uint16_t)(buf - scratch);
        }

        addr   = scratch;
        length = config_desc->wTotalLength;
    } else if (type == USB_DESC_STRING && index == USB_STRING_LANGUAGE_ID) {
        APPEND_DESC(&usb_langid);

        addr   = scratch;
        length = scratch[0];
    } else if (type == USB_DESC_STRING && index - 1 < set->string_count) {
        __code const char *string = set->strings[index - 1];

        *buf++ = 2;               // bLength
        *buf++ = USB_DESC_STRING; // bDescriptorType

        while (*string) {
            *buf++ = *string++;
            *buf++ = 0;
            scratch[0] += 2;
        }

        addr   = scratch;
        length = scratch[0];
    } else if (type == USB_DESC_CLASS_REPORT) {
        uint8_t iface_index = req->wIndex;

        if (iface_index == 0) {
            length = sizeof(hid_report_desc_keyboard);
            APPEND(&hid_report_desc_keyboard, length);
        } else if (iface_index == 1) {
            length = sizeof(hid_report_desc_extra);
            APPEND(&hid_report_desc_extra, length);
        } else {
            STALL_EP0();
            return;
        }
    } else if (type == USB_DESC_CLASS_HID) {
        uint8_t iface_index = req->wIndex;

        if (iface_index == 0) {
            APPEND_DESC(&usb_desc_hid_main);

            addr   = scratch;
            length = scratch[0];
        } else if (iface_index == 1) {
            APPEND_DESC(&usb_desc_hid_extra);

            addr   = scratch;
            length = scratch[0];
        } else {
            STALL_EP0();
            return;
        }
    } else {
        STALL_EP0();
        return;
    }

    uint16_t __xdata received_length = req->wLength;
    // truncate data to the allowed lengths received from the host
    length = (received_length < length) ? received_length : length;
    setup_ep0_in_xfer(addr, length);
    step_ep0_in_xfer();

    SET_EP0_IN_RDY;
}

static void usb_get_configuration_handler()
{
    usb_ep0_state = USB_EP0_STATE_RECV_STATUS;

    EP0_IN_BUF[0] = active_configuration;

    SET_EP0_CNT(1);
    SET_EP0_IN_RDY;
}

static void usb_get_interface_handler()
{
    usb_ep0_state = USB_EP0_STATE_RECV_STATUS;

    EP0_IN_BUF[0] = 0;

    SET_EP0_CNT(1);
    SET_EP0_IN_RDY;
}

static void usb_hid_get_report_handler()
{
    STALL_EP0();
}

static void usb_hid_set_report_handler(__xdata struct usb_req_setup *req)
{
    switch (req->wValue >> 8) {
        case REPORT_TYPE_OUTPUT:
            if ((req->wIndex == 0) && (req->wLength == 0x0001)) {
                usb_ep0_state = USB_EP0_STATE_LED;
                SET_EP0_OUT_RDY;
            }

            break;

        case REPORT_TYPE_FEATURE:
            if ((req->wValue & 0xff) == REPORT_ID_ISP) {
                usb_ep0_state = USB_EP0_STATE_ISP;
                SET_EP0_OUT_RDY;
            }

            break;

        default:
            STALL_EP0();
            break;
    }
}

static void usb_hid_set_idle_handler(__xdata struct usb_req_setup *req)
{
    // TODO: finish implementaiton to use the set idle time
    idle_time = req->wValue >> 8;

    CLEAR_EP0_CNT;
    SET_EP0_IN_RDY;
}

static void usb_hid_get_idle_handler()
{
    EP0_IN_BUF[0] = idle_time;

    SET_EP0_CNT(1);
    SET_EP0_IN_RDY;
}

static void usb_hid_set_protocol_handler(__xdata struct usb_req_setup *req)
{
    if (req->wIndex == 0) {
        interface0_protocol = req->wValue & 0xff;
    } else if (req->wIndex == 1) {
        interface1_protocol = req->wValue && 0xff;
    }

    CLEAR_EP0_CNT;
    SET_EP0_IN_RDY;
}

static void usb_hid_get_protocol_handler(__xdata struct usb_req_setup *req)
{
    if (req->wIndex == 0) {
        EP0_IN_BUF[0] = interface0_protocol;
    } else if (req->wIndex == 1) {
        EP0_IN_BUF[0] = interface1_protocol;
    }

    SET_EP0_CNT(1);
    SET_EP0_IN_RDY;
}

void usb_ep0_out_irq()
{
    if (usb_ep0_state == USB_EP0_STATE_LED) {
        usb_ep0_state = 0;

        keyboard_state.led_state = EP0_OUT_BUF[0];

        CLEAR_EP0_CNT;
        SET_EP0_IN_RDY;
    } else if (usb_ep0_state == USB_EP0_STATE_ISP) {
        usb_ep0_state = 0;

        if (EP0_OUT_BUF[0] == 0x05 && EP0_OUT_BUF[1] == 0x75) {
            isp_jump();
        }
    } else {
        usb_ep0_state = 0;
        CLEAR_EP0_CNT;
        SET_EP0_IN_RDY;
    }
}

void usb_ep0_in_irq()
{
    if (usb_ep0_state == USB_EP0_STATE_IN_DATA) {
        step_ep0_in_xfer();

        SET_EP0_IN_RDY;
        SET_EP0_OUT_RDY;
    } else if (usb_ep0_state == USB_EP0_STATE_RECV_STATUS) {
        SET_EP0_OUT_RDY;
        SET_EP0_IN_STALL;
    } else {
        SET_EP0_IN_STALL;
        SET_EP0_OUT_RDY;

        USBADDR = received_usb_addr;
    }
}

static void setup_ep0_in_xfer(uint8_t *src, uint16_t len)
{
    ep0_xfer_src        = src;
    ep0_xfer_bytes_left = len;
}

static void step_ep0_in_xfer()
{
    if (ep0_xfer_bytes_left == 0) {
        usb_ep0_state = USB_EP0_STATE_RECV_STATUS;

        SET_EP0_CNT(0);
    } else if (ep0_xfer_bytes_left > EP0_BUF_SIZE) {
        usb_ep0_state = USB_EP0_STATE_IN_DATA;
        set_ep0_in_buffer(ep0_xfer_src, EP0_BUF_SIZE);
        ep0_xfer_bytes_left -= EP0_BUF_SIZE;
        ep0_xfer_src += EP0_BUF_SIZE;

        SET_EP0_CNT(EP0_BUF_SIZE);
    } else if (ep0_xfer_bytes_left == EP0_BUF_SIZE) {
        usb_ep0_state = USB_EP0_STATE_IN_DATA;
        set_ep0_in_buffer(ep0_xfer_src, EP0_BUF_SIZE);
        ep0_xfer_bytes_left = 0;

        SET_EP0_CNT(EP0_BUF_SIZE);
    } else {
        usb_ep0_state = USB_EP0_STATE_RECV_STATUS;
        set_ep0_in_buffer(ep0_xfer_src, ep0_xfer_bytes_left);

        SET_EP0_CNT(ep0_xfer_bytes_left);
    }
}

static void set_ep0_in_buffer(uint8_t *src, uint8_t len)
{
    if (len > EP0_BUF_SIZE) {
        dprintf("%s buffer too long: %d", __func__, len);
        return;
    }

    memcpy(EP0_IN_BUF, src, len);
}

static void get_ep0_out_buffer(uint8_t *dest)
{
    memcpy(dest, EP0_OUT_BUF, EP0_BUF_SIZE);
}

static void set_ep1_in_buffer(uint8_t *src, uint8_t len)
{
    if (len > EP1_BUF_SIZE) {
        dprintf("%s buffer too long: %d", __func__, len);
        return;
    }

    memcpy(EP1_IN_BUF, src, len);
}

static void get_ep1_out_buffer(uint8_t *dest)
{
    memcpy(dest, EP1_OUT_BUF, EP1_BUF_SIZE);
}
