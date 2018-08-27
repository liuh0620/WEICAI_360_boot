#!/bin/bash

if [ "$1" == "imx6" ]; then
#source /home/zhangd/gcc/environment-setup-cortexa9hf-vfp-neon-poky-linux-gnueabi
source /home/VideoStorage/liuh/cross_compile/gcc/environment-setup-cortexa9hf-vfp-neon-poky-linux-gnueabi 
make clean
make
cp 360_boot /home/VideoStorage/liuh/work/output/
sync
fi
if [ "$1" == "t3" ]; then
make -f Makefile_T3 clean
make -f Makefile_T3
fi
