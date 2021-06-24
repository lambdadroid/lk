# lk2nd
lk2nd is a bootloader for Qualcomm MSM devices (at the moment only MSM8916 and MSM8939),
based on the [CodeAurora Little Kernel fork](https://source.codeaurora.org/quic/la/kernel/lk/).
It provides an Android Fastboot interface on devices where the stock bootloader
does not provide Fastboot (e.g. Samsung).

On MSM8916 it is also used for some quirks for mainline devices, e.g. to set
a proper WiFi/BT MAC address in the device tree.

lk2nd does not replace the stock bootloader. It is packaged into an Android
boot image and then loaded by the stock bootloader as a "secondary" bootloader.
The real Android boot image is placed into the boot partition with 512 KiB offset,
and then loaded by lk2nd.

## Supported SoCs
- `msm8916-secondary`: MSM8216, MSM8916, MSM8929, MSM8939
- `msm8974-secondary`: MSM8974
- `msm8226-secondary`: MSM8226, MSM8926

See [Chipsets](https://github.com/efidroid/projectmanagement/wiki/%5BReference%5D-Chipsets)
page on the EFIDroid wiki for an exact mapping of LK targets to SoCs.

### Supported devices
- Alcatel OneTouch Idol 3 (4.7) - 6039*
- Alcatel OneTouch Idol 3 (5.5) - 6045*
- Asus Zenfone 2 Laser (720p) - Z00L
- Asus Zenfone 2 Laser (1080p) - Z00T
- Asus Zenfone Max ZC550KL (2016) - Z010D
- BQ Aquaris X5 - paella, picmt
- DragonBoard 410c - apq8016-sbc
- HTC One M8s - m8qlul (quirky - see comment in `dts/msm8916/msm8939-htc-m8qlul.dts`)
- Huawei Ascend G7 - G7-L01
- Huawei Honor 5X - kiwi
- Lenovo A6000
- Lenovo A6010
- Lenovo PHAB Plus - PB1-770M, PB1-770N
- LG K10 (m216) - K420
- LG Leon LTE (c50) - H340, H342, H343, H345, MS345, etc
- LYF Wind 1 - ls5010
- Marshall London
- Motorola Moto E (2015) - surnia
- Motorola Moto G (2015) - osprey
- Motorola Moto G4 Play - harpia
- Samsung Galaxy A3 (2015) - SM-A300FU
- Samsung Galaxy A5 (2015) - SM-A500F, SM-A500FU
- Samsung Galaxy Core Prime LTE - SM-G360F
- Samsung Galaxy Grand Prime - SM-G530W
- Samsung Galaxy J3 (2016) - SM-J3109
- Samsung Galaxy J3 Pro - SM-J3110, SM-J3119
- Samsung Galaxy J5 (2015) - SM-J5008, SM-J500F, SM-J500FN, SM-J500H
- Samsung Galaxy J5 (2016) - SM-J5108, SM-J510F, SM-J510FN
- Samsung Galaxy J7 (2015) - SM-J7008, SM-J700P
- Samsung Galaxy On7 (2015) - SM-G6000
- Samsung Galaxy S4 Mini Value Edition - GT-I9195I
- Samsung Galaxy S5 - SM-G900F
- Samsung Galaxy Tab 4 10.1 (2015) - SM-T533
- Samsung Galaxy Tab A 8.0 LTE (2015) - SM-T357W
- Samsung Galaxy Tab A 9.7 WiFi (2015) - SM-T550
- SONY Xperia Z1 / Z1 Compact / Z Ultra - rhine
- Vodafone Smart prime 6
- Wileyfox Swift - crackling
- Xiaomi Mi 4i - ferrari
- Xiaomi Redmi 2 - wt86047, wt88047

## Installation
1. Download `lk2nd.img` (available in [Releases](https://github.com/msm8916-mainline/lk2nd/releases))
2. Flash `lk2nd.img` using the stock flashing procedure:
  - Fastboot: `fastboot flash boot lk2nd.img`
  - Samsung: `heimdall flash --BOOT lk2nd.img`

If you get `fastboot: error: Couldn't parse partition size '0x'` try one of the following workarounds:

- `fastboot flash:raw boot lk2nd.img`
- `fastboot boot lk2nd.img`, then `fastboot flash lk2nd lk2nd.img`

## Usage
lk2nd provides the standard Android fastboot protocol for flashing/booting Android boot images.

Press `Volume Down` while booting to enter Fastboot mode.
Press `Volume Up` while booting to boot into Recovery mode.

**Note:** If your stock bootloader uses the same key combinations, you need to wait a bit before
pressing the volume keys. Usually, waiting until the screen turns on and/or the device vibrates
should be enough to make the stock bootloader ignore the keys.

`fastboot flash lk2nd lk2nd.img` can be used to update lk2nd directly from its
fastboot interface.

**Note:** `fastboot flash boot boot.img` will flash the actual boot image with 512 KiB offset
into the boot partition. This is done to avoid replacing lk2nd (since it is also booted from
the boot partition).

Other fastboot commands work normally.

### Troubleshooting
If the device shows up via fastboot you can get a log file from lk2nd using
`fastboot oem lk_log && fastboot get_staged <output-file>`, where `<output-file>`
is either some text file to write to (e.g. `output.txt`) or `/dev/stdout` to
write the log to standard output.

## Building
Check [Supported SoCs](#supported-socs) for the make target you should use below.
(It depends on the SoC of your device.)

```
$ make TOOLCHAIN_PREFIX=arm-none-eabi- msmXXXX-secondary
```

**Requirements:**
- ARM (32 bit) GCC tool chain
  - Arch Linux: `arm-none-eabi-gcc`
- [Device Tree Compiler](https://git.kernel.org/pub/scm/utils/dtc/dtc.git)
  - Arch Linux: `dtc`

Replace `TOOLCHAIN_PREFIX` with the path to your tool chain.
`lk2nd.img` is built and placed into `build-msm8916-secondary/lk2nd.img`.

## Porting
### To other MSM8916/8974 devices
- Add a simple device tree to `dts/`. You just need `model` and the
  `qcom,msm-id`/`qcom,board-id` from downstream.

### To other SoCs
Qualcomm maintains separate branches for various groups of SoCs. The branches can
be seen on the [Chipsets](https://github.com/efidroid/projectmanagement/wiki/%5BReference%5D-Chipsets)
page on the EFIDroid wiki. This version of lk2nd is based on the LA.BR branch for
MSM8916. There is a [fork for MSM8953 based on the LA.UM branch](https://github.com/msm8953-mainline/lk2nd).

The bootloader will work best when you use the correct branch for your device.
Older platforms are usually kept around by Qualcomm but barely tested and may
not work, or not even compile.

However, if make files for your SoC are present in this version or the MSM8953 fork
you can try to enable it and see if it works well enough for you. Otherwise you
would need to go through the Git history and pick the relevant commits to another
branch from https://source.codeaurora.org/quic/la/kernel/lk/.

To enable support for a SoC that is already present in this repository:

1. Create a new `project/<target>-secondary.mk` which looks like the others.
2. Try to compile it and fix all the compile errors.
3. Try to run it and hope that it works.

Good luck!

## Contact
Ping `minecrell`/`Mis012` on [`#postmarketos-mainline`](https://wiki.postmarketos.org/wiki/Matrix_and_IRC).
