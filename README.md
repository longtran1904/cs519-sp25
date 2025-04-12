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

### Load kernel and use QEMU

Compile kernel and load kernel to qemu directory:
```
cd linux-5.15.0
../qemu/compile_kernel_qemu.sh
```

Run QEMU with new kernel image:
```
cd $BASE
./qemu/run_qemu.sh
```




