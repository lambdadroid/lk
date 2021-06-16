#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-only
set -eux

nproc && grep Mem /proc/meminfo && df -hT .
apk add build-base dtc dtc-dev gcc-arm-none-eabi python3 tar
make -j$(nproc) TOOLCHAIN_PREFIX=arm-none-eabi- "$1"
