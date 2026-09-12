# CIY X77

## Specs

- MCU: SH68F89
- Layout: TKL (ANSI 87)
- Matrix: 6 rows x 18 columns
- Backlight: per-key RGB, driven from the four PCA units
- Indicators: Num, Caps and Scroll Lock LEDs on P3.0-P3.2
- Settings store: an external 24Cxx I2C EEPROM, not the MCU's own data block
- Wireless: None
- USB: `258a:0016`

## SMK Supported Features

- [x] Key Scan
- [x] Lock LEDs
- [x] Settings persistence
- [ ] RGB Matrix

## Fn Layer

- `Fn`+`F1`-`F4` - my computer, web search, calculator, media select
- `Fn`+`F5`-`F8` - previous, next, play/pause, stop
- `Fn`+`F9`-`F11` - mute, volume down, volume up
- `Fn`+`F12` - keyboard lock, drops every key until pressed again
- `Fn`+`Gui` - Gui lock, drops Gui and App
- `Fn`+`ScrLk` - NKRO toggle
- `Fn`+`W` - WASD swap, both ways, so the arrows type WASD

## Matrix

Columns are driven low one at a time: 0-7 on P0, 8-15 on P1 and 16-17 on P4.6/P4.7. Rows
read back on P5.0-P5.5, active low. The columns are on SFR page 0 and the rows on page 1,
so every scan crosses the page boundary.

The stock firmware ships one keymap for the whole board family, so its matrix is wider
than this unit's. Two groups of its positions are not fitted on an ANSI TKL:

- **Columns 18-21 on P7.1-P7.4** are a numpad. `MATRIX_COLS` is 18 and P7 is left an input,
  since those pins may not even be routed here. A full-size unit would want them back.
- **The ISO and JIS extras**, which the stock keymap fills with Yen at `(1,13)`, NonUS hash
  at `(3,12)` and `(4,12)`, Ro at `(4,11)`, NonUS backslash at `(4,14)`,
  Kana/Henkan/Muhenkan at `(5,3)`, `(5,6)`, `(5,7)` and `(5,11)`, Hanja at `(5,4)` and a
  right Gui at `(5,13)`. They are `KC_NO` here. On an ISO unit the NonUS backslash at
  `(4,14)` and the NonUS hash at `(3,12)` are the two to bring back.

All 18 columns and all 6 rows are confirmed on hardware.

## Settings storage

The board carries a second memory chip: a 24Cxx-class EEPROM on a bit-banged bus, SDA on
P5.6, SCL on P5.7 and write protect on P4.1. SDA and SCL share port 5 with the matrix
rows, so the row read has to mask them off or an idle-low bus reads as a pressed key.

The SH68F89's own 2 KB EEPROM-like block is left untouched, which is also what the stock
firmware does.

## Code Options

This firmware requires the following (stock) code options that are programmed on the
SH68F89 in the CIY X77:

```
Code Options: b8c0038c

OP_WDT      1 - Enable WDT function
OP_WDTPD    0 - Disable WDT function in Power-Down mode
OP_RST      1 - Pin 7.0 used as I/O pin
OP_LVREN    1 - Enable LVR function
OP_LVRLEVEL 2 - 2.7V LVR Level 3
OP_SCM      0 - SCM invalid in warm-up period
OP_SINK     0 - P0/P1 sink ability normal mode
OP_DRIVE    0 - P3/P4 driver ability normal mode
OP_OSCRFB   0 - External oscillator feedback resistance 2M ohm
OP_OSC      3 - Internal RC 128 KHz + internal RC 12 MHz
OP_ISP      1 - Disable ISP function
OP_ISPPIN   0 - Enter ISP mode directly regardless of P3.4/P3.5
OP_OVL      0 - OVL generates WDT Reset
OP_WMT      3 - Shortest warm-up time
OP_OSCDRV   3 - 8M~12M crystal
OP_DACTIME  0 - 100 us
```

`OP_WDT` is enabled, so any loop that can run for more than the watchdog period has to
kick it. `OP_OSC` selects the internal 12 MHz RC, which is what the PLL multiplies to the
48 MHz the USB block needs and the 24 MHz SYSCLK.
