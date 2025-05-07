#/bin/bash -x
sudo apt update; sudo apt-get install -y libdpkg-dev kernel-package libncurses-dev

PROC=`nproc`
export CONCURRENCY_LEVEL=$PROC
export CONCURRENCYLEVEL=$PROC

# sudo cp /boot/config-$(uname -r) .config
# make menuconfig

scripts/config --disable SYSTEM_TRUSTED_KEYS
scripts/config --disable SYSTEM_REVOCATION_KEYS

# make oldconfig
make menuconfig

touch REPORTING-BUGS
#sudo make clean -j
#sudo make prepare
sudo make -j$PROC
sudo make modules -j$PROC
sudo make modules_install
sudo make install

y="5.15.178"

sudo cp ./arch/x86/boot/bzImage /boot/vmlinuz-$y
sudo cp System.map /boot/System.map-$y
sudo cp .config /boot/config-$y
sudo update-initramfs -c -k $y

sudo update-grub
