#!bin/sh
set -x
mknod /dev/h3_ioctl_test_device c 229 121
./user_ioctl
./user_ioctl -c 1
./user_ioctl -c 1
./user_ioctl
./user_ioctl -c 4 2 open_reg
./user_ioctl -c 16 1 1
./user_ioctl -c 16 0 1
./user_ioctl
./user_ioctl -c 4 1 open_reg
./user_ioctl -c 4 1 read_reg
./user_ioctl
./user_ioctl -c 8 2 open_reg
./user_ioctl -c 8 1 open_reg
./user_ioctl
./user_ioctl -c 8 1 read_reg
./user_ioctl -c 2 1
./user_ioctl
./user_ioctl -c 2 2
./user_ioctl

