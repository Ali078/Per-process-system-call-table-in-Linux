#include "sys_clone2_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <asm/unistd.h>

#ifndef __NR_clone2
#error clone2 system call not defined
#endif


int sys_clone2_wrapper(int (*fn)(void), void *stackTop,
			int flags, void *arg, short int vector_id)
{
	long 		pid;
	long 		*temp = NULL;
	int		p_tid = 0;
	int 		c_tid = 0;
	unsigned long 	tls_val = 0;

	if (fn == NULL || stackTop == NULL) {
		return -EINVAL;
	}

	temp = stackTop;
	*temp = (long)fn;
	/*printf("%ld,%ld\n", stackTop, temp);*/

	pid = syscall(__NR_clone2, flags, stackTop,
		      &p_tid, &c_tid, tls_val, vector_id);

	if (pid < 0) {
		errno = pid;
		return -1;
	}
	/*printf("post syscall values p_tid = 0x%x c_tid = 0x%x\n",p_tid,c_tid);*/
	return (int)pid;
}

