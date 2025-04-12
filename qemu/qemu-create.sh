#!/bin/bash

sudo apt install -y qemu-system-x86
#Please do not change beyond this


#Now create a disk for your virtual machine 
#for 20GB
if [ ! -f "$QEMU_IMG_FILE" ]; then
    qemu-img create $QEMU_IMG_FILE 32g
fi

#Now format your disk with some file system; 
#ext4 in this example
sudo mkfs.ext4 $QEMU_IMG_FILE
sudo chown -R $USER $QEMU_IMG_FILE

#Now create a mount point directory for your image file
mkdir $QEMU_DIR

#Next, mount your image to the directory
sudo mount -o loop $QEMU_IMG_FILE $QEMU_DIR

#Install debootstrap
sudo apt-get install debootstrap

#Now get the OS release version using
# cat /etc/os-release

#Set family name 
sudo debootstrap --arch amd64 $OS_RELEASE_NAME $QEMU_DIR

#Chroot and Now install all your required packages; lets start with vim and build_esstentials.
sudo chroot $QEMU_DIR /bin/bash 
#You are all set. Now unmount your image file from the directory.
exit