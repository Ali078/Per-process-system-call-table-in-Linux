#ifndef _SYSCALL_TBL_H
#define _SYSCALL_TBL_H

#include <linux/list.h>
#include <linux/module.h>
#include <asm/unistd.h>

#include <asm/syscall_tbl_api.h>

#include "list.h"

#define MAX_SYSCALL_TBL_SIZE 10
#define MAX_SYSCALL_VECTOR_SIZE 10

#define MAX_PDE_LENGTH 64
#define MAX_NAME_LENGTH 64

#define NUM_SYSCALLS ((int) ((600) / 8)) + 1 

struct syscall {
	char name[MAX_NAME_LENGTH];
	unsigned long fptr;
	short syscall_num;
	struct module * module;
} typedef syscall_t;


struct syscall_vector {
	short id;
	short refcnt;
	char blocked[NUM_SYSCALLS];
	node_t syscall_num_list;
} typedef syscall_vector_t;

// has a list of vectors
// has a list of syscalls registered
struct syscall_tbl {
	spinlock_t lock;
	node_t vector_list;
	node_t syscall_list;	
} typedef syscall_tbl_t;

struct syscall_vector_info {
        short vector_id;
        int size;
        char syscalls[20][MAX_NAME_LENGTH];
} typedef syscall_vector_info_t;

struct syscall_tbl_info {
        int size;
	syscall_vector_info_t  syscall_vector_info[10];
} typedef syscall_tbl_info_t;


/*

node_t * get_syscall(long fptr);
node_t * get_syscall_vector(short id);
node_t * get_syscall_of_syscall_vector(node_t * syscall_num_list, short syscall_num);


int register_syscall(unsigned long fptr, short syscall_num, struct module * module);
// check for refcount of module
int unregister_syscall(unsigned long fptr);

// create syscall vector, add to tbl
// copies the list from inherit_vector_id, 
// 	leaves the list empty if SYSCALL_NO_INHERIT is passed as paramter : see syscall_tbl_api.h
int add_syscall_vector(short inherit_vector_id);
// remove syscall vector from tbl - check ref count
int remove_syscall_vector(short vector_id);
// add a syscall to vector : syscall identified by fptr, check in syscall_list in syscall_tbl, inc module ref count of syscall
int add_syscall_to_vector(short vector_id, unsigned long fptr);
// remove syscall vector : dec ref count of module
int remove_syscall_from_vector(short vector_id, unsigned long fptr);

// cat syscall vectors in the tbl
int show_syscall_tbl(void);

// get syscall fn address to execute given syscall_num and vector_id
unsigned long get_syscall_fn(short syscall_num, short vector_id);

int assign_syscall_vector(short vector_id);
int unassign_syscall_vector(short vector_id);

static long cmp_syscall_by_fptr(node_t * first, node_t * second) {
        return XNODE_DATA(second, syscall_t *)->fptr - XNODE_DATA(first, syscall_t *)->fptr;
}


static long cmp_syscall_by_syscall_num(node_t * first, node_t * second) {
	return XNODE_DATA(second, syscall_t *)->syscall_num - XNODE_DATA(first, syscall_t *)->syscall_num;
}

static long cmp_syscall_vector(node_t * first, node_t * second) {
	return (long) XNODE_DATA(second, syscall_vector_t *)->id - (long) XNODE_DATA(first, syscall_vector_t *)->id;
}
*/

#endif
