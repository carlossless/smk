<div align="center">
  <img src="https://github.com/carlossless/smk/assets/498906/30535a69-47a5-4229-8e08-fe2a840d8355" alt="SMK" />
</div>

# SMK - Small (device) Mechanical Keyboard Firmware

[![Build](https://github.com/carlossless/smk/actions/workflows/build.yml/badge.svg)](https://github.com/carlossless/smk/actions/workflows/build.yml) [![](https://img.shields.io/badge/discord-SMK-blue)](https://discord.gg/SZFBDBuxrK)

This is a keyboard firmware similar to [QMK](https://github.com/qmk/qmk_firmware), but targeting 8051-based devices like the Sinowealth SH68F90A (labeled as BYK916 or BYK901). It's aimed to be at least partially compatible with QMK configurations.

The S (Small) in SMK comes from this firmware using [SDCC](https://sdcc.sourceforge.net/) to build itself.

## ⚠️ WARNING ⚠️

This firmware is still highly experimental, so be cautious when trying to use it or extend it.

You can very easily end up with a bricked device if the written firmware can't jump back into ISP mode, so before testing or modifying it, it's best to have a full dump of your stock firmware and a tool (like the Sinolink) that can write it back.

## Supported Devices

| Keyboard | MCU | ISP | USB | Wireless | Details |
| -------- | --- | --- | --- | -------- | ------- |
| [NuPhy Air60 v1](https://nuphy.com/products/air60) | SH68F90A / BYK916 | ✅ | ✅ | 2.4G (BT WIP) | [Details](docs/keyboards/nuphy-air60.md) |
| E-YOOSO Z11 | SH68F90A / BYK901 | ✅ | ✅ | N/A | [Details](docs/keyboards/nuphy-air60.md) |
| Royal Kludge RK61 | SH68F90? / BYK901 | ✅ | ✅ | N/A | [Details](docs/keyboards/royalkludge-rk61.md) |

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
* [sinowealth-kb-tool](https://github.com/carlossless/sinowealth-kb-tool) latest version - required only for flashing

### Building & Flashing

Once all prerequisites are set up, you can build and flash firmware for a specific combination of keyboard and layout using the following commands:

```sh
meson setup build # configure meson build dir
meson compile -C build nuphy-air60_default_smk.hex # build firmware for nuphy-air60 with the default layout
meson compile -C build nuphy-air60_default_flash # write firmware to the device via sinowealth-kb-tool
```

## Acknowledgements

* [libfx2](https://github.com/whitequark/libfx2)
* [LUFA](https://github.com/abcminiuser/lufa)
* [QMK](https://github.com/qmk/qmk_firmware)
