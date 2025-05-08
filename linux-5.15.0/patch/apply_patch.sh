#!/bin/bash
patch -p0 < fair.patch
patch -p0 < syscall_tbl.patch
patch -p0 < syscalls.patch