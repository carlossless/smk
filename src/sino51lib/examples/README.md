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

Each links six or seven modules and comes to under a kilobyte.

## Pins and board configuration

The on-chip blocks sit on pins the part fixes, not pins a board chooses: see the part's
`spi_hw.h`, `twi_hw.h` and `uart_hw.h`, and the tables in `docs/platforms/`. `gpio_blink`
picks P3.0 arbitrarily and says so.

Where a driver reads its configuration from `kbdef.h` — the SPI clock divider, the TWI bit
rate — the example ships a `kbdef.h` of its own holding what a board would set.

## What is not here, and why

`usbhw.c`, `systick.c` and `power.c` cannot be exercised on their own, because they call into
the firmware rather than the other way round: the USB device stack, the tick dispatcher and
`usb_init`/`usb_deinit` all live in `src/smk`. Using those three today means taking that stack
with them, which is why there is no `usb` example. `diag.c` is in the same position through
`dprintf`.
