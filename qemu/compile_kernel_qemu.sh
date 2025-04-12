#! /bin/bash

PARA=16
VER=5.15.0

set -x

#cd kernel/kbuild

#make -f Makefile.setup .config
#make -f Makefile.setup
# make -j$PARA
#sudo make modules
#sudo make modules_install
#sudo make install

mkdir -p $QEMU_KERNEL_DIR

cd $KERNEL
# sudo cp def.config .config
sudo make -j$PARA

sudo cp ./arch/x86/boot/bzImage $QEMU_KERNEL_DIR/vmlinuz-$VER
sudo cp System.map $QEMU_KERNEL_DIR/System.map-$VER

set +x
