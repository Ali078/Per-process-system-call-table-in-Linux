obj-m += sys_clone2.o

INC=/lib/modules/$(shell uname -r)/build/arch/x86/include

all: test0_clone2 test1_clone2 test2_clone2 clone2

test0_clone2: test0_clone2.c
	gcc -I$(INC)/generated/uapi -I$(INC)/uapi test0_clone2.c sys_clone2_lib.c -o test0_clone2

test1_clone2: test1_clone2.c
	gcc -I$(INC)/generated/uapi -I$(INC)/uapi test1_clone2.c sys_clone2_lib.c -o test1_clone2

test2_clone2: test2_clone2.c
	gcc -I$(INC)/generated/uapi -I$(INC)/uapi test2_clone2.c sys_clone2_lib.c -o test2_clone2

clone2:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f  test0_clone2 test1_clone2 test2_clone2
