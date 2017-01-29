obj-m += syscall_tbl_mod.o test.o test_syscalls.o test_vectors.o  hw3_ioctl.o

syscall_tbl_mod-objs := syscall_tbl.o list.o

INC=/lib/modules/$(shell uname -r)/build/arch/x86/include

all: modules test_clone2

modules:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

test_clone2: test_clone2.c
	gcc -I$(INC)/generated/uapi -I$(INC)/uapi test_clone2.c ../clone2/sys_clone2_lib.c -o test_clone2

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f  test_clone2
