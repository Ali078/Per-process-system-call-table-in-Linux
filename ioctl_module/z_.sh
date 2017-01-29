#!bin/sh
set -x
lsmod
rmmod hw3_ioctl
insmod hw3_ioctl.ko
lsmod
