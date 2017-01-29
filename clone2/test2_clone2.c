#define _GNU_SOURCE
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "../ioctl_module/user_ioctl.h"
#include <sys/types.h>
#include <sys/syscall.h>

#define gettid() syscall(SYS_gettid);

#define CLONE_SYSCALLS 0x00001000

#define STACK_SIZE (1024 * 1024)    /* Stack size for cloned child */

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
			} while (0)
struct myargs {
	short vector_id;
	char *name;
	pid_t pid;
	syscall_tbl_info_t *sys;
} typedef myargs;


static int childFunclevel1()
{
	char ioctl_device_location[] = "/dev/hw3_ioctl_test_device";
	int fd;
	int rc = -1;

	myargs send_arg;
	send_arg.vector_id = 0;
	send_arg.name = NULL;
	send_arg.pid = gettid();

	printf("id of child1 %d\n", send_arg.pid);
	/*printf("tid of child1 %d\n", getpid());*/

	fd = open(ioctl_device_location, O_RDONLY);
	if (fd < 0) {
		printf("Is hw3_ioctl_device module loaded?\n");
		return -EACCES;
	}

	printf("Inside Child function level 1\n");

	rc = ioctl(fd, IOCTL_SYSCALL_TBL_GET_VECTOR_ID_OF_PROCESS, &send_arg);
	printf("Vector id of process id = %d Vector ID = %d\n", send_arg.pid, send_arg.vector_id);

	sleep(1);

	/* child-level0 terminates now */
	printf("Terminating now Child at level 1\n");
	exit(EXIT_SUCCESS);
	return 0;
}


static int childFunclevel0()
{
	char *stack;                    /* Start of stack buffer */
	char *stackTop;                 /* End of stack buffer */
	int pid;
	short int vector_id = 0xFF;
	char ioctl_device_location[] = "/dev/hw3_ioctl_test_device";
	int fd;
	int rc = -1;

	myargs send_arg;
	send_arg.vector_id = 0;
	send_arg.name = NULL;
	send_arg.pid = gettid();

	printf("pid of child0 %d\n", send_arg.pid);

	/*printf("tid of child0 %d\n", getpid());*/

	fd = open(ioctl_device_location, O_RDONLY);
	if (fd < 0) {
		printf("Is hw3_ioctl_device module loaded?\n");
		return -EACCES;

	}

	printf("Inside Child function level 0\n");

	rc = ioctl(fd, IOCTL_SYSCALL_TBL_GET_VECTOR_ID_OF_PROCESS, &send_arg);
	printf("Vector id of process id = %d Vector ID = %d\n", send_arg.pid, send_arg.vector_id);

	/* Allocate stack for child */
	stack = malloc(STACK_SIZE);
	if (stack == NULL)
		errExit("malloc");
	stackTop = stack + STACK_SIZE;

	/* Create child that has its own UTS namespace;
	child commences execution in childFunc() */
	pid = sys_clone2_wrapper(childFunclevel1, stackTop, SIGCHLD, NULL, vector_id);

	if (pid == -1)
		errExit("clone");
	printf("clone() returned pid = %ld\n", (long) pid);

	sleep(2);

	if (waitpid(pid, NULL, 0) == -1)    /* Wait for child */
		errExit("waitpid");

	printf("Terminating now Child at level 0\n");

	exit(EXIT_SUCCESS);
	return 0;
}

int main(void)
{
	char *stack;                    /* Start of stack buffer */
	char *stackTop;                 /* End of stack buffer */
	int pid;
	short int vector_id = 0x01;
	char ioctl_device_location[] = "/dev/hw3_ioctl_test_device";
	int fd;
	int rc = -1;

	myargs send_arg;
	send_arg.vector_id = 0;
	send_arg.name = NULL;
	send_arg.pid = gettid();

	printf("id of parent %d\n", send_arg.pid);
	/*printf("tid of parent %d\n", getpid());*/

	fd = open(ioctl_device_location, O_RDONLY);
	if (fd < 0) {
		printf("Is hw3_ioctl_device module loaded?\n");
		return -EACCES;
	}

	printf("Inside Parent Process\n");

	rc = ioctl(fd, IOCTL_SYSCALL_TBL_GET_VECTOR_ID_OF_PROCESS, &send_arg);
	printf("Vector id of process id = %d Vector ID = %d\n", send_arg.pid, send_arg.vector_id);

	/* Allocate stack for child */
	stack = malloc(STACK_SIZE);
	if (stack == NULL)
		errExit("malloc");
	stackTop = stack + STACK_SIZE;

	/* Create child that has its own UTS namespace;
	child commences execution in childFunc() */
	pid = sys_clone2_wrapper(childFunclevel0, stackTop, SIGCHLD | CLONE_SYSCALLS, NULL, vector_id);

	if (pid == -1)
		errExit("clone");
	printf("clone() returned pid = %ld\n", (long) pid);

	sleep(3);

	if (waitpid(pid, NULL, 0) == -1)    /* Wait for child */
		errExit("waitpid");
	printf("Terminating now parent\n");
	exit(EXIT_SUCCESS);
}
