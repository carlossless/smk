# Genesis Thor 300

## Specs

- MCU: [SH68F881](../platforms/sh68f881.md)
- Layout: TKL (ISO, tall Enter)
- Matrix: 6 rows x 19 columns
- Backlight: single colour LEDs, one per key, sharing the key matrix
- Indicators: Caps Lock and Scroll Lock LEDs
- Wireless: None
- USB: `258a:001f`

## SMK Supported Features

- [x] Key Scan
- [x] LED Matrix
- [x] Settings persistence

## Fn Layer

The secondary legends printed on the caps:

- `Fn`+`F1`-`F4` - my computer, web search, calculator, media select
- `Fn`+`F5`-`F8` - previous, next, play/pause, stop
- `Fn`+`F9`-`F11` - mute, volume down, volume up
- `Fn`+`F12` - keyboard lock, drops every key until pressed again
- `Fn`+`Gui` - Gui lock, drops Gui and App
- `Fn`+`ScrLk` - NKRO toggle
- `Fn`+`W` - WASD swap, both ways, so the arrows type WASD
- `Fn`+`1`-`5` - select a backlight animation directly, `5` turns it off
- `Fn`+`Ins` - cycle to the next animation
- `Fn`+`Up`/`Down` - backlight brightness
- `Fn`+`Left`/`Right` - animation speed

`Fn`+`6`-`0` are unmapped: stock offers ten lighting modes where this firmware has four
animations plus off, and aliasing the rest onto duplicates would be misleading.

Brightness dithers whole frames rather than dimming with PWM, because the columns are
plain outputs with no timer behind them.

## Backlight

One column is driven low per subframe with up to six anodes high, so a whole frame is 19
subframes. The anodes must idle low: left high they clamp the row lines through the
backlight and every column reads as pressed.

## Code Options

This firmware requires the following (stock) code options that are programmed on the
SH68F881 in the Genesis Thor 300

```
Code Options: 20c00082

OP_WDT      0 - Enable WDT function
OP_WDTPD    0 - Disable WDT function in Power-Down mode
OP_RST      1 - P4.7 used as I/O pin
OP_WMT      0 - longest warm up time
OP_CRMC     0 - Other oscillator types exclude 400k-2M ceramic is used
OP_LVREN    1 - Enable LVR function
OP_LVRLE    1 - 3.1V LVR level 2
OP_SCM      0 - SCM is invalid in warm up period
OP_OSC      0 - Oscillator1 is internal 12M RC
OP_ISP      1 - Disable ISP function
OP_ISPPIN   0 - Enter ISP mode only when P8.6 and P8.7 are connected to GND, simultaneously
OP_OVL      0 - OVL generates WDT Reset
OP_OSCDRV   0 - minimum
OP_RCUSBCAL 1 - Enable USB calibration
OP_RC32KCAL 0 - Disable 32.768KHz crystal calibration
```

`OP_WDT` is enabled, so any loop that can run for more than the watchdog period has to
kick it.
