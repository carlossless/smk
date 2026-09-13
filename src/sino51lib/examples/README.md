# sino51lib examples

Standalone firmware that uses the library and nothing else. Each one lists exactly the
modules it links, so a dependency that creeps from the library into the rest of the tree
shows up here as a link error rather than years later.

Build them all:

```sh
meson compile -C build examples
```

which leaves `ex-<example>-<part>.hex` in the build directory. They are not installed and no
keyboard uses them.

| example | parts | exercises |
| --- | --- | --- |
| `gpio_blink` | all | `reset_init`, `ldo_init`, `clock_init`, `peripherals_init`, the GPIO macros, `delay_ms`, `watchdog_kick` |
| `uart_hello` | all | the EUART: baud from `FREQ_SYS`, `uart_putc`, the transmit-complete ISR |
| `spi_xfer` | all | hardware SPI master, full duplex, divider checked against the part's SPR ladder |
| `twi_probe` | sh68f89 | hardware TWI master: START, address, STOP, walking the address space |
| `pwm_duty` | sh68f90 | the PWM banks through `pwm.h`'s duty macros |
| `pca_duty` | sh68f89 | the PCA units as an 8-bit PWM bank, with `pca_hold`/`pca_release` around a reload |
| `flash_info` | all | reading the factory information block through the FAC window |
| `usb_minimal` | all | `usbhw.c`: the SIE brought up, EP0 answered well enough to enumerate |
| `systick_tick` | all | `systick.c`: Timer2 as the periodic tick, both slot lengths |

Each links six or seven modules and comes to under two kilobytes.

## Hooks

Two of the library's modules call outwards rather than only being called:

| module | wants | supplied by |
| --- | --- | --- |
| `usbhw.c` | `usb_irq_dispatch()` | the firmware's USB device stack, or `usb_minimal/main.c` |
| `systick.c` | `tick_dispatch()` | the firmware's scheduler, or `systick_tick/main.c` |

Each is one function behind a header the user of the library owns. `src/smk` has its own
`usb.h` and `tick.h`; the examples have their own, a few lines each, and link none of the
firmware.

## Pins and board configuration

The on-chip blocks sit on pins the part fixes, not pins a board chooses: see the part's
`spi_hw.h`, `twi_hw.h` and `uart_hw.h`, and the tables in `docs/platforms/`. `gpio_blink`
picks P3.0 arbitrarily and says so.

Where a driver reads its configuration from `kbdef.h` — the SPI clock divider, the TWI bit
rate — the example ships a `kbdef.h` of its own holding what a board would set.

## What is not here, and why

`power.c` needs `usb_init`, `usb_deinit` and `usb_suspended`, which is a larger surface than a
hook: parking and restoring a USB device across a power-down is the device stack's business,
not the library's. An example would also have to sleep, and on the SH68F881 nothing but an
external interrupt wakes it, so it would need a board.

`diag.c` prints through `dprintf`, so it belongs to whatever owns the debug output.

Nothing else in the library reaches into `src/smk`, and the examples are built with `src/smk`
off the include path, so anything that starts to will fail here rather than quietly compiling.
