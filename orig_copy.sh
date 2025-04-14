#!/bin/bash

BASE=$PWD
echo $BASE
ORIGINAL=$PWD/linux-5.15.0
TARGET=$PWD/cs519-sp25/linux-5.15.0
PATCH=$PWD/cs519-sp25/patch

if [ -f include/linux/syscalls.h.orig ]; then
        echo "File include/linux/syscalls.h.orig backed up successfully."
fi
if [ -f arch/x86/entry/syscalls/syscall_64.tbl.orig ]; then
        echo "File arch/x86/entry/syscalls/syscall_64.tbl.orig backed up successfully."
fi
if [ -f mm/memory.c.orig ]; then
        echo "File mm/memory.c.orig backed up successfully."
fi
if [ -f mm/Makefile.orig ]; then
        echo "File mm/Makefile.orig backed up successfully."
fi

cd $TARGET
cp $ORIGINAL/include/linux/syscalls.h include/linux/syscalls.h.orig
cp $ORIGINAL/arch/x86/entry/syscalls/syscall_64.tbl arch/x86/entry/syscalls/syscall_64.tbl.orig
cp $ORIGINAL/mm/memory.c mm/memory.c.orig
cp $ORIGINAL/mm/Makefile mm/Makefile.orig
if [ -f include/linux/syscalls.h ]; then
        echo "Transfered file include/linux/syscalls.h successfully."
fi
if [ -f arch/x86/entry/syscalls/syscall_64.tbl ]; then
        echo "Transfered arch/x86/entry/syscalls/syscall_64.tbl successfully."
fi
if [ -f mm/memory.c ]; then
        echo "Transfered mm/memory.c successfully."
fi
if [ -f mm/Makefile ]; then
        echo "Transfered mm/Makefile successfully."
fi

diff -u $TARGET/include/linux/syscalls.h.orig $TARGET/include/linux/syscalls.h > $PATCH/syscalls.patch
diff -u $TARGET/arch/x86/entry/syscalls/syscall_64.tbl.orig $TARGET/arch/x86/entry/syscalls/syscall_64.tbl > $PATCH/syscall_64.patch
diff -u $TARGET/mm/memory.c.orig $TARGET/mm/memory.c > $PATCH/memory.patch
diff -u $TARGET/mm/Makefile.orig $TARGET/mm/Makefile > $PATCH/Makefile.patch
diff -u /dev/null $TARGET/mm/extent.h > $PATCH/extent.patch
diff -u /dev/null $TARGET/mm/extent.c > $PATCH/extent.c.patch



