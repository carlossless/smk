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

| PCB | MCU | Wireless IC | JTAG Setup |
| --- | --- | ----------- | ---------- |
| ![top](https://github.com/carlossless/smk/assets/498906/43ad50b2-6666-424d-94f8-ca8e9207eb7b) | ![mcu](https://github.com/carlossless/smk/assets/498906/295a8904-d131-45e1-a5dd-d2938b1a116b) | ![wireless-ic](https://github.com/carlossless/smk/assets/498906/ab980bf1-6123-4947-b05b-c006d6e41ef0) |

## SMK Supported Features

- [x] Key Scan
- [x] RGB Matrix
- [ ] Wireless

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
