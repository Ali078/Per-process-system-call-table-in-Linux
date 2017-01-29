#ifndef SYS_CLONE2_LIB
#define SYS_CLONE2_LIB

#include <stdio.h>
#include <stdlib.h>

int sys_clone2_wrapper(int (*fn)(void), void *stackTop,
			int flags, void *arg, short int vector_id);

#endif
