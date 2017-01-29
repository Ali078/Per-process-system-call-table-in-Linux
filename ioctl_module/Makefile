IOCTL_VERSION = "0.1"

obj-m += hw3_ioctl.o

all: clean user_ioctl

user_ioctl: user_ioctl.c
	gcc -Wall -Werror user_ioctl.c -o user_ioctl
clean:
	rm -rf user_ioctl
