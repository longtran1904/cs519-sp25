#### Coding and other resources for CS 519

CS 519 Instructions
------------------

### QEMU Set Up
First time set up cloudlab, run:
`sudo apt update`

Always run this command to use qemu scripts:
`source ./qemu/setvars.sh`

### First time set up QEMU:
```
./qemu/qemu-create.sh
```
This script will drop you to the disk image shell.
Install system core services & packages
```
apt update
apt install -y bash coreutils vim net-tools iputils-ping
apt install -y login passwd init systemd systemd-sysv
```

Install gdb for user-space program debug
`apt install gdb`

Set password for the virtualized ubuntu disk image
`passwd`

Exit the disk image shell
`exit`

### To apply the patch on a new kernel

Download a new kernel from source
```
rm -rf linux-5.15.0 # Remove modified kernel
sudo apt-get update
sudo sed -i 's/# deb-src/deb-src/' /etc/apt/sources.list
sudo apt update
apt source linux-image-unsigned-$(uname -r)
```

Apply patches to the new kernel
`./apply_batch`

### Compile kernel for the first time
```
cd linux-5.15.0
../project1/install_packages.sh
../project1/compile_os_nopackages.sh
```

### Load kernel and use QEMU

Remember to setvar before using QEMU scripts:
```
cd linux-5.15.0
../qemu/setvars.sh
```

Compile kernel and load kernel to qemu directory:
```
cd linux-5.15.0
../qemu/compile_kernel_qemu.sh
```

Run QEMU with new kernel image:
(This script mount file disk, recompile project1/test and copy into file disk, then run QEMU image)
```
../qemu/run_qemu.sh
```

### In QEMU, how to test user-space program:
login using username: root - password you set before
```
cd /
./test
```

If you want to see details of extents:
`dmesg`

To exit:
`Ctrl+A X`

if you want to change user-space test program, change test.c in `project1/`, then run QEMU again `../qemu/run_qemu.sh`