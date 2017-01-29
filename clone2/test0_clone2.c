#define _GNU_SOURCE
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
			} while (0)

int function_call;

/* Start function for cloned child */
static int childFunc()
{
	char childname[] = "Child_Process  ";
	struct utsname uts;

	childname[14] = (char)((char)function_call + (char)'0');

	printf("Inside Child function\n");

	/* Change hostname in UTS namespace of child */
	if (sethostname(childname, strlen(childname)) == -1) {
		errExit("sethostname");
	}

	/* Retrieve and display hostname */
	if (uname(&uts) == -1) {
	errExit("uname");
	}
	printf("uts.nodename in child:  %s\n", uts.nodename);

	/* Keep the namespace open for a while, by sleeping.
	*  This allows some experimentation--for example, another
	*  process might join the namespace. */
	sleep(1);

	printf("Terminating Now child %d\n", function_call);
	/* child terminates now */
	exit(EXIT_SUCCESS);
	return 0;
}

#define STACK_SIZE (1024 * 1024)    /* Stack size for cloned child */

int main(void)
{
	char *stack;                    /* Start of stack buffer */
	char *stackTop;                 /* End of stack buffer */
	int pid1, pid2;
	struct utsname uts;
	short int vector_id = 0x01;

	printf("Inside Parent Process\n");

	sleep(1);
	/* Allocate stack for child */
	stack = malloc(STACK_SIZE);
	if (stack == NULL)
		errExit("malloc");
	stackTop = stack + STACK_SIZE;  /* Assume stack grows downward */

	/* Create child that has its own UTS namespace;
	*child commences execution in childFunc() */

	function_call++;
	pid1 = sys_clone2_wrapper(childFunc, stackTop, CLONE_NEWUTS | SIGCHLD, NULL, vector_id);
	if (pid1 == -1)
		errExit("clone");
	printf("clone() returned pid1 = %ld\n", (long) pid1);


	sleep(2);
	function_call++;
	pid2 = clone(childFunc, stackTop, CLONE_NEWUTS | SIGCHLD, NULL);
	if (pid2 == -1)
		errExit("clone");
	printf("clone() returned pid2 = %ld\n", (long) pid2);

	/* Parent falls through to here */
	sleep(2);
	/* Display hostname in parent's UTS namespace. This will be
	different from hostname in child's UTS namespace. */
	if (uname(&uts) == -1)
		errExit("uname");
	printf("uts.nodename in parent: %s\n", uts.nodename);

	if (waitpid(pid1, NULL, 0) == -1)    /* Wait for child */
		errExit("waitpid");

	if (waitpid(pid2, NULL, 0) == -1)    /* Wait for child */
		errExit("waitpid");

	printf("Terminating now Parent\n");
	exit(EXIT_SUCCESS);
}
