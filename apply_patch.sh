
BASE=$PWD
echo "BASE=$BASE"
PATCH=$BASE/cs519-sp25/patch
TARGET=$BASE/linux-5.15.0

patch -p3 < $PATCH/syscalls.patch
patch -p3 < $PATCH/syscall_64.patch
patch -p3 < $PATCH/memory.patch
patch -p3 < $PATCH/Makefile.patch
patch -p3 < $PATCH/extent.patch
patch -p3 < $PATCH/extent.c.patch