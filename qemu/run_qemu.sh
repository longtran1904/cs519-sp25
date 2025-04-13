#!/bin/bash
set -x

$BASE/qemu/unmount_qemu.sh
sleep 1
#Launching QEMU

#$APPBENCH/scripts/killqemu.sh

# #sudo qemu-system-x86_64 -kernel $KERNEL/vmlinuz-$VER -hda $QEMU_IMG_FILE -append "root=/dev/sda rw" --curses -m $QEMUMEM -smp cores=16 -cpu host -enable-kvm -device e1000,netdev=net0 -netdev user,id=net0,hostfwd=tcp::5555-:22 -net nic -nographic -serial mon:stdio
# # sudo qemu-system-x86_64 -kernel $QEMU_KERNEL_DIR/vmlinuz-$VER -hda $QEMU_IMG_FILE -append "root=/dev/sda rw console=tty0 console=ttyS0,9600n8" -m $QEMUMEM -smp cores=4 -cpu host -enable-kvm -device e1000,netdev=net0 -netdev user,id=net0,hostfwd=tcp::5555-:22 -net nic -nographic -serial mon:stdio
# # sudo qemu-system-x86_64 -kernel $QEMU_KERNEL_DIR/vmlinuz-$VER -hda $QEMU_IMG_FILE -append "root=/dev/sda rw console=tty0 console=ttyS0,9600n8" -m $QEMUMEM -smp cores=4 -cpu host -enable-kvm -device virtio-net-pci -nic user,model=virtio-net-pci,hostfwd=tcp::5555-:22 -nographic -serial mon:stdio
# sudo qemu-system-x86_64 \
#     -kernel $QEMU_KERNEL_DIR/vmlinuz-$VER \
#     -hda $QEMU_IMG_FILE \
#     -append "root=/dev/sda rw console=tty0 console=ttyS0,9600n8" \
#     -m $QEMUMEM \
#     -smp cores=4 \
#     -cpu host \
#     -enable-kvm \
#     -nographic -serial mon:stdio
# # -device e1000,netdev=net0 \
# #     -netdev user,id=net0,hostfwd=tcp::5555-:22 \

### Update user-space program to Disk Image
sudo mount -o loop $QEMU_IMG_FILE $QEMU_DIR

cd $BASE/project1
rm -f test
# Compile test user-space program
gcc -o test -g test.c
sudo cp test $QEMU_DIR/test

sudo umount $QEMU_DIR

sleep 3

### Run QEMU
cd $BASE
sudo qemu-system-x86_64 -kernel $QEMU_KERNEL_DIR/vmlinuz-$VER -hda $QEMU_IMG_FILE -append "root=/dev/sda rw console=tty0 console=ttyS0,9600n8" -m $QEMUMEM -smp cores=16 -cpu host -enable-kvm -device e1000,netdev=net0 -netdev user,id=net0,hostfwd=tcp::5555-:22 -net nic -nographic -serial mon:stdio

exit