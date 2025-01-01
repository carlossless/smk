# NuPhy Air60 

## Specs

- MCU: BYK916 (SH68F90A)
- Backlight: RGB LEDs
- Indicators: 2x5 RGB LEDs (on each side, part of the RGB backlight matrix)
- Switches:
    - 2-state switch to control OS mode (win/mac)
    - 3-state switch to control power and usb/wireless mode (off/usb/wireless)
- Wireless: BK3632 (supports BT and 2.4G)

## Pictures

| PCB | MCU | Wireless IC |
| --- | --- | ----------- |
| ![top](https://github.com/carlossless/smk/assets/498906/43ad50b2-6666-424d-94f8-ca8e9207eb7b) | ![mcu](https://github.com/carlossless/smk/assets/498906/295a8904-d131-45e1-a5dd-d2938b1a116b) | ![wireless-ic](https://github.com/carlossless/smk/assets/498906/ab980bf1-6123-4947-b05b-c006d6e41ef0) |

## SMK Supported Features

- [x] Key Scan
- [x] RGB Matrix
- [~] Wireless

## Code Options

This firmware requires the following (stock) code options that are programmed on the BYK916 (SH68F90A) in the E-YOOSO Z11

```
Code Option String: A4E063C00F000088
Code Option Number: 0x8800000fc063e0a4

Byte 0 - A4
OP_OSCDRIVE 2 - 4MHz crystal or 8~12MHz crystal with external capacitance(C1=C2)<20pF
OP_RST 1 - P0.2 used as IO pin
OP_WMT 0 - longest warm up time
OP_SCMEN 1 - Disable SCM
OP_OSCRFB 0 - 150K

Byte 1 - E0
OP_LVREN 1 - Enable LVR function
OP_LVRLEVEL 3 - 2.1V LVR level4
OP_SCM 0 - SCM is invalid in warm up period
OP_OSC2SEL 0 - OSC2 select 12M RC
OP_IOV1 0 - P7.1/P7.2/P7.3/P7.4 input/output level is VUSB(5V)
OP_IOV0 0 - P5.5/P5.6 input/output level is VUSB(5V)

Byte 2 - 63
OP_SCMSEL 3 - 8MHz
OP_OSC 3 - Oscillator1 is internal 128KHz RC, oscillator2 is internal 24MHz RC

Byte 3 - C0
OP_ISP 1 - Disable ISP function
OP_ISPPIN 1 - Enter ISP mode only when P6.3 and P6.4 are connected to GND, simultaneously
OP_WDT 0 - Enable WDT function
OP_WDTPD 0 - Disable WDT function in Power-Down mode

Byte 4 - 0F
OP_SINK1 3 - Port6[5:0] sink ability largest mode(380mA)
OP_SINK0 3 - P4.7/Port7[7:5] sink ability largest mode(50mA)

Byte 5 - 00
OP_BOPTP 0 - (1+21%)tr min
OP_BOPTN 0 - (1+21%)tf min

Byte 6 - 00
Unused

Byte 7 - 88
OP_ISPSIZE 8 - 0Bytes
```

## Vendor HID Descriptors

### USB

#### 1

```
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x06,        // Usage (Keyboard)
0xA1, 0x01,        // Collection (Application)
0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
0x19, 0xE0,        //   Usage Minimum (0xE0)
0x29, 0xE7,        //   Usage Maximum (0xE7)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x01,        //   Logical Maximum (1)
0x95, 0x08,        //   Report Count (8)
0x75, 0x01,        //   Report Size (1)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x95, 0x01,        //   Report Count (1)
0x75, 0x08,        //   Report Size (8)
0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x95, 0x05,        //   Report Count (5)
0x75, 0x01,        //   Report Size (1)
0x05, 0x08,        //   Usage Page (LEDs)
0x19, 0x01,        //   Usage Minimum (Num Lock)
0x29, 0x05,        //   Usage Maximum (Kana)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x95, 0x01,        //   Report Count (1)
0x75, 0x03,        //   Report Size (3)
0x91, 0x01,        //   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
0x19, 0x00,        //   Usage Minimum (0x00)
0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
0x95, 0x05,        //   Report Count (5)
0x75, 0x08,        //   Report Size (8)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0xFF,        //   Usage Page (Reserved 0xFF)
0x09, 0x03,        //   Usage (0x03)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0xC0,        //   Usage Page (Reserved 0xC0)
0x09, 0x00,        //   Usage (0x00)
0x15, 0x80,        //   Logical Minimum (-128)
0x25, 0x7F,        //   Logical Maximum (127)
0x95, 0x40,        //   Report Count (64)
0x75, 0x08,        //   Report Size (8)
0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection

// 89 bytes
```

#### 2

```
0x06, 0x01, 0x00,  // Usage Page (Generic Desktop Ctrls)
0x09, 0x80,        // Usage (Sys Control)
0xA1, 0x01,        // Collection (Application)
0x85, 0x01,        //   Report ID (1)
0x19, 0x81,        //   Usage Minimum (Sys Power Down)
0x29, 0x83,        //   Usage Maximum (Sys Wake Up)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x01,        //   Logical Maximum (1)
0x95, 0x03,        //   Report Count (3)
0x75, 0x01,        //   Report Size (1)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x95, 0x01,        //   Report Count (1)
0x75, 0x05,        //   Report Size (5)
0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
0x05, 0x0C,        // Usage Page (Consumer)
0x09, 0x01,        // Usage (Consumer Control)
0xA1, 0x01,        // Collection (Application)
0x85, 0x02,        //   Report ID (2)
0x19, 0x00,        //   Usage Minimum (Unassigned)
0x2A, 0xFF, 0x02,  //   Usage Maximum (0x02FF)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x7F,  //   Logical Maximum (32767)
0x95, 0x01,        //   Report Count (1)
0x75, 0x10,        //   Report Size (16)
0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
0x09, 0x01,        // Usage (0x01)
0xA1, 0x01,        // Collection (Application)
0x85, 0x03,        //   Report ID (3)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x09, 0x2F,        //   Usage (0x2F)
0x75, 0x08,        //   Report Size (8)
0x95, 0x03,        //   Report Count (3)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x06,        // Usage (Keyboard)
0xA1, 0x01,        // Collection (Application)
0x85, 0x04,        //   Report ID (4)
0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
0x19, 0x04,        //   Usage Minimum (0x04)
0x29, 0x70,        //   Usage Maximum (0x70)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x01,        //   Logical Maximum (1)
0x75, 0x01,        //   Report Size (1)
0x95, 0x78,        //   Report Count (120)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
0x09, 0x01,        // Usage (0x01)
0xA1, 0x01,        // Collection (Application)
0x85, 0x05,        //   Report ID (5)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x19, 0x01,        //   Usage Minimum (0x01)
0x29, 0x02,        //   Usage Maximum (0x02)
0x75, 0x08,        //   Report Size (8)
0x95, 0x05,        //   Report Count (5)
0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
0x09, 0x01,        // Usage (0x01)
0xA1, 0x01,        // Collection (Application)
0x85, 0x06,        //   Report ID (6)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x19, 0x01,        //   Usage Minimum (0x01)
0x29, 0x02,        //   Usage Maximum (0x02)
0x75, 0x08,        //   Report Size (8)
0x96, 0x07, 0x04,  //   Report Count (1031)
0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x02,        // Usage (Mouse)
0xA1, 0x01,        // Collection (Application)
0x85, 0x07,        //   Report ID (7)
0x09, 0x01,        //   Usage (Pointer)
0xA1, 0x00,        //   Collection (Physical)
0x05, 0x09,        //     Usage Page (Button)
0x15, 0x00,        //     Logical Minimum (0)
0x25, 0x01,        //     Logical Maximum (1)
0x19, 0x01,        //     Usage Minimum (0x01)
0x29, 0x05,        //     Usage Maximum (0x05)
0x75, 0x01,        //     Report Size (1)
0x95, 0x05,        //     Report Count (5)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x95, 0x03,        //     Report Count (3)
0x81, 0x01,        //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
0x16, 0x00, 0x80,  //     Logical Minimum (-32768)
0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
0x09, 0x30,        //     Usage (X)
0x09, 0x31,        //     Usage (Y)
0x75, 0x10,        //     Report Size (16)
0x95, 0x02,        //     Report Count (2)
0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
0x15, 0x81,        //     Logical Minimum (-127)
0x25, 0x7F,        //     Logical Maximum (127)
0x09, 0x38,        //     Usage (Wheel)
0x75, 0x08,        //     Report Size (8)
0x95, 0x01,        //     Report Count (1)
0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x0C,        //     Usage Page (Consumer)
0x0A, 0x38, 0x02,  //     Usage (AC Pan)
0x95, 0x01,        //     Report Count (1)
0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              //   End Collection
0xC0,              // End Collection

// 227 bytes
```

### Wireless Dongle

#### 1

```
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x06,        // Usage (Keyboard)
0xA1, 0x01,        // Collection (Application)
0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
0x19, 0xE0,        //   Usage Minimum (0xE0)
0x29, 0xE7,        //   Usage Maximum (0xE7)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x01,        //   Logical Maximum (1)
0x75, 0x01,        //   Report Size (1)
0x95, 0x08,        //   Report Count (8)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x95, 0x01,        //   Report Count (1)
0x75, 0x08,        //   Report Size (8)
0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x08,        //   Usage Page (LEDs)
0x19, 0x01,        //   Usage Minimum (Num Lock)
0x29, 0x05,        //   Usage Maximum (Kana)
0x95, 0x05,        //   Report Count (5)
0x75, 0x01,        //   Report Size (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x95, 0x01,        //   Report Count (1)
0x75, 0x03,        //   Report Size (3)
0x91, 0x01,        //   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
0x19, 0x00,        //   Usage Minimum (0x00)
0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
0x95, 0x05,        //   Report Count (5)
0x75, 0x08,        //   Report Size (8)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0xFF,        //   Usage Page (Reserved 0xFF)
0x09, 0x03,        //   Usage (0x03)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection

// 73 bytes
```

#### 2

```
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x02,        // Usage (Mouse)
0xA1, 0x01,        // Collection (Application)
0x09, 0x01,        //   Usage (Pointer)
0xA1, 0x00,        //   Collection (Physical)
0x85, 0x01,        //     Report ID (1)
0x05, 0x09,        //     Usage Page (Button)
0x19, 0x01,        //     Usage Minimum (0x01)
0x29, 0x05,        //     Usage Maximum (0x05)
0x15, 0x00,        //     Logical Minimum (0)
0x25, 0x01,        //     Logical Maximum (1)
0x95, 0x05,        //     Report Count (5)
0x75, 0x01,        //     Report Size (1)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x95, 0x01,        //     Report Count (1)
0x75, 0x03,        //     Report Size (3)
0x81, 0x01,        //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
0x09, 0x30,        //     Usage (X)
0x09, 0x31,        //     Usage (Y)
0x16, 0x00, 0x80,  //     Logical Minimum (-32768)
0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
0x75, 0x10,        //     Report Size (16)
0x95, 0x02,        //     Report Count (2)
0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              //   End Collection
0xA1, 0x00,        //   Collection (Physical)
0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
0x09, 0x38,        //     Usage (Wheel)
0x15, 0x81,        //     Logical Minimum (-127)
0x25, 0x7F,        //     Logical Maximum (127)
0x75, 0x08,        //     Report Size (8)
0x95, 0x01,        //     Report Count (1)
0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              //   End Collection
0xA1, 0x00,        //   Collection (Physical)
0x05, 0x0C,        //     Usage Page (Consumer)
0x0A, 0x38, 0x02,  //     Usage (AC Pan)
0x95, 0x01,        //     Report Count (1)
0x75, 0x08,        //     Report Size (8)
0x15, 0x81,        //     Logical Minimum (-127)
0x25, 0x7F,        //     Logical Maximum (127)
0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              //   End Collection
0xC0,              // End Collection
0x06, 0x01, 0xFF,  // Usage Page (Vendor Defined 0xFF01)
0x09, 0x00,        // Usage (0x00)
0xA1, 0x01,        // Collection (Application)
0x85, 0x02,        //   Report ID (2)
0x09, 0x00,        //   Usage (0x00)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x75, 0x08,        //   Report Size (8)
0x95, 0x07,        //   Report Count (7)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x06,        // Usage (Keyboard)
0xA1, 0x01,        // Collection (Application)
0x85, 0x07,        //   Report ID (7)
0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
0x19, 0xE0,        //   Usage Minimum (0xE0)
0x29, 0xE7,        //   Usage Maximum (0xE7)
0x25, 0x01,        //   Logical Maximum (1)
0x75, 0x01,        //   Report Size (1)
0x95, 0x08,        //   Report Count (8)
0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x95, 0x30,        //   Report Count (48)
0x75, 0x01,        //   Report Size (1)
0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
0x19, 0x00,        //   Usage Minimum (0x00)
0x29, 0x2F,        //   Usage Maximum (0x2F)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x06,        // Usage (Keyboard)
0xA1, 0x01,        // Collection (Application)
0x85, 0x08,        //   Report ID (8)
0x95, 0x38,        //   Report Count (56)
0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
0x19, 0x30,        //   Usage Minimum (0x30)
0x29, 0x67,        //   Usage Maximum (0x67)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x06,        // Usage (Keyboard)
0xA1, 0x01,        // Collection (Application)
0x85, 0x09,        //   Report ID (9)
0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
0x19, 0x68,        //   Usage Minimum (0x68)
0x29, 0x9F,        //   Usage Maximum (0x9F)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
0x06, 0x02, 0xFF,  // Usage Page (Vendor Defined 0xFF02)
0x09, 0x02,        // Usage (0x02)
0xA1, 0x01,        // Collection (Application)
0x85, 0x06,        //   Report ID (6)
0x09, 0x02,        //   Usage (0x02)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x75, 0x08,        //   Report Size (8)
0x95, 0x07,        //   Report Count (7)
0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
0x05, 0x0C,        // Usage Page (Consumer)
0x09, 0x01,        // Usage (Consumer Control)
0xA1, 0x01,        // Collection (Application)
0x85, 0x05,        //   Report ID (5)
0x26, 0x3C, 0x02,  //   Logical Maximum (572)
0x19, 0x00,        //   Usage Minimum (Unassigned)
0x2A, 0x3C, 0x02,  //   Usage Maximum (AC Format)
0x75, 0x10,        //   Report Size (16)
0x95, 0x01,        //   Report Count (1)
0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x80,        // Usage (Sys Control)
0xA1, 0x01,        // Collection (Application)
0x85, 0x03,        //   Report ID (3)
0x19, 0x81,        //   Usage Minimum (Sys Power Down)
0x29, 0x83,        //   Usage Maximum (Sys Wake Up)
0x25, 0x01,        //   Logical Maximum (1)
0x95, 0x03,        //   Report Count (3)
0x75, 0x01,        //   Report Size (1)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x95, 0x01,        //   Report Count (1)
0x75, 0x05,        //   Report Size (5)
0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection

// 254 bytes
```
