#!/bin/bash
set -x

$BASE/qemu/unmount_qemu.sh
sleep 1

### Update user-space program to Disk Image
sudo mount -o loop $QEMU_IMG_FILE $QEMU_DIR

cd $BASE/project3
### Compile test user-space program
rm -f multi-bench
gcc -o multi-bench multi-bench.c -lpthread
sudo cp multi-bench $QEMU_DIR/multi-bench

sudo umount $QEMU_DIR

sleep 3

### Run QEMU
cd $BASE
sleep 3
sudo qemu-system-x86_64 -kernel $QEMU_KERNEL_DIR/vmlinuz-$VER -hda $QEMU_IMG_FILE -append "root=/dev/sda rw console=tty0 console=ttyS0,9600n8" -m $QEMUMEM -smp cores=16 -cpu host -enable-kvm -device e1000,netdev=net0 -netdev user,id=net0,hostfwd=tcp::5555-:22 -net nic -nographic -serial mon:stdio

exit