make clean
make
rmmod test_vectors
rmmod test_syscalls
rmmod syscall_tbl_mod
rmmod hw3_ioctl 
lsmod
insmod syscall_tbl_mod.ko
cd ../clone2
make
sh install_module.sh
cd ../syscall_module
insmod test_syscalls.ko
insmod test_vectors.ko
insmod hw3_ioctl.ko
mknod /dev/hw3_ioctl_test_device c 229 121
lsmod
