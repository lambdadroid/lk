# lk2nd
lk2nd is a bootloader for Qualcomm MSM devices (at the moment only MSM8916 and MSM8939),
based on the [CodeAurora Little Kernel fork](https://source.codeaurora.org/quic/la/kernel/lk/).
It provides an Android Fastboot interface on devices where the stock bootloader
does not provide Fastboot (e.g. Samsung).

On MSM8916 it is also used for some quirks for mainline devices, e.g. to set
a proper WiFi/BT MAC address in the device tree.

lk2nd does not replace the stock bootloader. It is packaged into an Android
boot image and then loaded by the stock bootloader as a "secondary" bootloader.
The real Android boot image is placed into the boot partition with 1 MB offset,
and then loaded by lk2nd.

## Supported SoCs
- MSM8216
- MSM8916
- MSM8929
- MSM8939

### Supported devices
- Alcatel OneTouch Idol 3 (5.5) - 6045*
- Marshall London
- Motorola Moto G4 Play (harpia)
- Samsung Galaxy A3 (2015) - SM-A300FU
- Samsung Galaxy A5 (2015) - SM-A500F, SM-A500FU
- Samsung Galaxy J3 (2016) - SM-J3109
- Samsung Galaxy J3 Pro - SM-J3110, SM-J3119
- Samsung Galaxy J5 (2015) - SM-J5008, SM-J500F, SM-J500FN, SM-J500H
- Samsung Galaxy J5 (2016) - SM-J5108, SM-J510F, SM-J510FN
- Samsung Galaxy J7 (2015) - SM-J7008, SM-J700P
- Samsung Galaxy S4 Mini Value Edition - GT-I9195I
- Samsung Galaxy Tab 4 10.1 (2015) - SM-T533
- Samsung Galaxy Tab A 8.0 LTE (2015) - SM-T357W
- Samsung Galaxy Tab A 9.7 WiFi (2015) - SM-T550
- Vodafone Smart prime 6
- Wileyfox Swift

## Installation
1. Download `lk2nd.img` (available in [Releases](https://github.com/msm8916-mainline/lk2nd/releases))
2. Flash `lk2nd.img` using the stock flashing procedure:
  - Fastboot: `fastboot flash boot lk2nd.img`
  - Samsung: `heimdall flash --BOOT lk2nd.img`

## Usage
lk2nd provides the standard Android fastboot protocol for flashing/booting Android boot images.

Press `Volume Down` while booting to enter Fastboot mode.
Press `Volume Up` while booting to boot into Recovery mode.

**Note:** If your stock bootloader uses the same key combinations, you need to wait a bit before
pressing the volume keys. Usually, waiting until the screen turns on and/or the device vibrates
should be enough to make the stock bootloader ignore the keys.

`fastboot flash lk2nd lk2nd.img` can be used to update lk2nd directory from its
fastboot interface.

**Note:** `fastboot flash boot boot.img` will flash the actual boot image with 1 MB offset
into the boot partition. This is done to avoid replacing lk2nd (since it is also booted from
the boot partition).

Other fastboot commands work normally.

## Building
```
$ make TOOLCHAIN_PREFIX=arm-none-eabi- msm8916-secondary
```

**Requirements:**
- ARM (32 bit) GCC tool chain
  - Arch Linux: `arm-none-eabi-gcc`
- [Device Tree Compiler](https://git.kernel.org/pub/scm/utils/dtc/dtc.git)
  - Arch Linux: `dtc`
  - Debian: `device-tree-compiler` (/usr/bin/dtc) and `libfdt-dev` are required

Replace `TOOLCHAIN_PREFIX` with the path to your tool chain.
`lk2nd.img` is built and placed into `build-msm8916-secondary/lk2nd.img`.

## Porting
### To other MSM8916 devices
- Add a simple device tree to `dts/`. You just need `model` and the
  `qcom,msm-id`/`qcom,board-id` from downstream.

### To other SoCs
- Cherry-pick changes
- Make some changes

(TODO: Document this properly)

## Contact
Ping `minecrell`/`Mis012` on [`#postmarketos-mainline`](https://wiki.postmarketos.org/wiki/Matrix_and_IRC).
