#!/bin/bash

#编译uboot
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- distclean #清除
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- mx6ull_alientek_emmc_defconfig #导入配置
make V=1 ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j16