#!/bin/bash
set -x
mkdir -p $BASE/mountdir
#Next, mount your image to the directory
sudo mount -o loop $QEMU_IMG_FILE $QEMU_DIR
