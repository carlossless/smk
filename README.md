<div align="center">
  <img src="https://github.com/carlossless/smk/assets/498906/30535a69-47a5-4229-8e08-fe2a840d8355" alt="SMK" />
</div>

# SMK - Small (device) Mechanical Keyboard Firmware

[![Build](https://github.com/carlossless/smk/actions/workflows/build.yml/badge.svg)](https://github.com/carlossless/smk/actions/workflows/build.yml) [![](https://img.shields.io/badge/discord-SMK-blue)](https://discord.gg/SZFBDBuxrK)

This is a keyboard firmware similar to [QMK](https://github.com/qmk/qmk_firmware), but targeting 8051-based devices like the SinoWealth SH68F90A (labeled as BYK916 or BYK901).

The S (Small) in SMK comes from this firmware using [SDCC](https://sdcc.sourceforge.net/) to build itself.

## ⚠️ WARNING ⚠️

This firmware is still highly experimental, so be cautious when trying to use it or extend it.

You can very easily end up with a bricked device if the written firmware can't jump back into ISP mode, so before testing or modifying it, it's best to have a full dump of your stock firmware and a programming tool (like an Arduino Nano + [sinodude-serial](https://github.com/carlossless/sinodude/tree/main/firmware) or SinoWealth SinoLink + ProWriter) that can write it back.

## Supported Devices

| Keyboard | MCU | ISP | USB | Wireless | Details |
| -------- | --- | --- | --- | -------- | ------- |
| [NuPhy Air60 v1](https://nuphy.com/products/air60) | SH68F90A / BYK916 | ✅ | ✅ | 2.4G (BT WIP) | [Details](docs/keyboards/nuphy-air60.md) |
| E-YOOSO Z11 | SH68F90A / BYK901 | ✅ | ✅ | N/A | [Details](docs/keyboards/nuphy-air60.md) |

## Developing

### Prerequisites

#### Nix

Currently, this project is primarily developed with the help of [Nix](https://nixos.org/) and Nix flakes. Please consider using Nix and the provided [flake](https://github.com/carlossless/smk/blob/master/flake.nix) to automatically set up a reproducible development environment.

With Nix installed and flakes enabled, use `nix develop` or [direnv](https://direnv.net/) to enter a shell with all prerequisites installed.

#### Manual

If setting up prerequisites without nix, you will need the following tools installed and available within your environment:

* [sdcc](https://sdcc.sourceforge.net/) >= 4.3.0
* [meson](https://mesonbuild.com/) >= 0.53
* [ninja](https://ninja-build.org/) >= 1.11.1
* [sinowisp](https://github.com/carlossless/sinowisp) latest version - required only for flashing

### Building & Flashing

A build directory targets one keyboard. Every supported keyboard has a cross-file under [`cross-file/`](cross-file) that names its MCU, USB IDs, LED matrix and radio, so configuring a build is just picking the right one:

```sh
meson setup build --cross-file cross-file/nuphy-air60.ini # configure for nuphy-air60
meson compile -C build                                    # build the firmware
meson compile -C build flash                              # write it to the device via sinowisp
```

The firmware lands in `build/nuphy-air60_default_smk.hex`. `meson setup` prints an `Unknown CPU family mcs51` warning because meson does not know the 8051 by name, and nothing depends on it recognising it.

A cross-file only supplies defaults, so you can still override any of them with `-Doption=value`, either while configuring or afterwards with `meson configure build -Doption=value`. To build several keyboards at once, give each one its own build directory.

If you are bringing up a new board or a different toolchain, configure by hand instead, using the toolchain-only cross-file:

```sh
meson setup build --cross-file cross-file/sdcc-mcs51.ini -Dkeyboard=nuphy-air60 -Dplatform=sh68f90
```

Run `meson configure build` to see every option and its current value. Omitting `-Dkeyboard` builds only the host-side tools.

## Acknowledgements

* [libfx2](https://github.com/whitequark/libfx2)
* [LUFA](https://github.com/abcminiuser/lufa)
* [QMK](https://github.com/qmk/qmk_firmware)
