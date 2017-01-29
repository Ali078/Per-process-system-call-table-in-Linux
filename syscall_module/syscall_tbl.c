#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>	
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/errno.h>

#include "syscall_tbl.h"

syscall_tbl_t * syscall_tbl = NULL;
struct module * SYSCALL_MODULE = NULL;

node_t * get_syscall(char * name);
node_t * get_syscall_vector(short id);
node_t * get_syscall_of_syscall_vector(node_t * syscall_num_list, short syscall_num);
int register_syscall(char * name, unsigned long fptr, short syscall_num, struct module * module);
int unregister_syscall(char * name);
int add_syscall_vector(void);
int remove_syscall_vector(short vector_id);
int add_syscall_to_vector(short vector_id, char * name);
int remove_syscall_from_vector(short vector_id, char * name);
int show_syscall_tbl(void);
int get_vector_by_pid(pid_t pid);
int assign_syscall_vector(short vector_id, pid_t pid);
int unassign_syscall_vector(pid_t pid);
int reassign_syscall_vector(short vector_id, pid_t pid);
int block_syscall_num(short vector_id, short syscall_num);
int unblock_syscall_num(short vector_id, short syscall_num);
unsigned long get_syscall_fn(short vector_id, short syscall_num);
int get_syscall_tbl_info(syscall_tbl_info_t * syscall_info);
int show_syscall_tbl_test(syscall_tbl_info_t * syscall_info); 


extern int (*_assign_syscall_vector)(short, pid_t);
extern int (*_unassign_syscall_vector)(pid_t);
extern unsigned long (*_get_syscall_fn)(short, short);

/*
static long cmp_syscall_by_fptr(node_t * first, node_t * second) {
	return XNODE_DATA(second, syscall_t *)->fptr - XNODE_DATA(first, syscall_t *)->fptr;
}
*/

static long cmp_syscall_by_name(node_t * first, node_t * second) {
	return strncmp(XNODE_DATA(second, syscall_t *)->name, XNODE_DATA(first, syscall_t *)->name, MAX_NAME_LENGTH);
}

static long cmp_syscall_by_syscall_num(node_t * first, node_t * second) {
	return XNODE_DATA(second, syscall_t *)->syscall_num - XNODE_DATA(first, syscall_t *)->syscall_num;
}

static long cmp_syscall_vector(node_t * first, node_t * second) {
	return (long) XNODE_DATA(second, syscall_vector_t *)->id - (long) XNODE_DATA(first, syscall_vector_t *)->id;
}

node_t * get_syscall(char * name) {
	node_t temp_node;
	syscall_t temp_syscall;
	if(strlen(name) > MAX_NAME_LENGTH - 1) return NULL; 
	temp_node.data = &temp_syscall;
	strncpy(temp_syscall.name, name, MAX_NAME_LENGTH - 1);
	return get_node(&temp_node, &syscall_tbl->syscall_list, cmp_syscall_by_name);
}

EXPORT_SYMBOL(get_syscall);

node_t * get_syscall_vector(short id) {
	node_t temp_node;
        syscall_vector_t temp_syscall_vector;
        temp_node.data = &temp_syscall_vector;
        temp_syscall_vector.id = id;
        return get_node(&temp_node, &syscall_tbl->vector_list, cmp_syscall_vector);
}

EXPORT_SYMBOL(get_syscall_vector);

node_t * get_syscall_of_syscall_vector(node_t * syscall_num_list, short syscall_num) {
        node_t temp_node;
	syscall_t syscall;
	syscall.syscall_num = syscall_num;
	temp_node.data = &syscall;
        return get_node(&temp_node, syscall_num_list, cmp_syscall_by_syscall_num);
}

EXPORT_SYMBOL(get_syscall_of_syscall_vector);

int is_syscall_blocked(short syscall_num, char * blocked) {
	char c = blocked[syscall_num / 8];
	return ((c >> (syscall_num % 8)) & 1);			
}

static int __add_syscall_vector(node_t * syscall_vector_node) {
	node_t * node = NULL, * tmp_node = NULL; int index = 0;
	struct list_head * item = NULL;
	if(list_empty(XNODE_LIST(&syscall_tbl->vector_list))) {
		XNODE_DATA(syscall_vector_node, syscall_vector_t *)->id = 1;
		add_node_end(syscall_vector_node, &syscall_tbl->vector_list);	
	} else {
		// takes next index if the first index is deleted
		list_for_each(item, XNODE_LIST(&syscall_tbl->vector_list)) {
			node = list_entry(item, node_t, entry);
			tmp_node = list_entry(XNODE_LIST(node)->next, node_t, entry);
			if(tmp_node != &syscall_tbl->vector_list && !list_is_last(XNODE_LIST(tmp_node), XNODE_LIST(&syscall_tbl->vector_list)) &&
				(XNODE_DATA(tmp_node, syscall_vector_t *)->id - XNODE_DATA(node, syscall_vector_t *)->id > 1)) { 
				break;
			}
		}
		index = XNODE_DATA(node, syscall_vector_t *)->id + 1;
		XNODE_DATA(syscall_vector_node, syscall_vector_t *)->id = index;
		add_node_end(syscall_vector_node, tmp_node);
	}
	return index;
}

static int __init init_syscall_tbl(void)
{
	int ret = 0;
        syscall_tbl = (syscall_tbl_t *)kmalloc(sizeof(syscall_tbl_t), GFP_KERNEL);
	if(!syscall_tbl) {
		printk("memory initialization failed\n");
		ret = -ENOMEM;
		goto err;
	}

	SYSCALL_MODULE = THIS_MODULE;

	spin_lock_init(&syscall_tbl->lock);
	INIT_LIST_HEAD(XNODE_LIST(&syscall_tbl->vector_list));
	INIT_LIST_HEAD(XNODE_LIST(&syscall_tbl->syscall_list));

	_assign_syscall_vector = assign_syscall_vector;
	_unassign_syscall_vector = unassign_syscall_vector;
	_get_syscall_fn = get_syscall_fn;

	goto out;
err:
	if(syscall_tbl) {
		kfree(syscall_tbl);
	}
	syscall_tbl = NULL;
	
out:
	return ret;
}


static void __exit exit_syscall_tbl(void)
{
	_assign_syscall_vector = NULL;
	_unassign_syscall_vector = NULL;
	_get_syscall_fn = NULL;
	SYSCALL_MODULE = NULL;
	kfree(syscall_tbl);	
	syscall_tbl = NULL;
}


int register_syscall(char * name, unsigned long fptr, short syscall_num, struct module * module) {
	int ret = 0;
	node_t * syscall_node = NULL;
	syscall_t * syscall = NULL;
	
	if(strlen(name) > MAX_NAME_LENGTH - 1) {
		printk("Name length greater than %d\n", MAX_NAME_LENGTH);
		ret = -EINVAL;
		goto err;
	}

	syscall_node = kmalloc(sizeof(node_t) + sizeof(syscall_t), GFP_KERNEL);
        if(!syscall_node) {
                printk("Memory initializtion failed\n");
                ret = -ENOMEM;
                goto err;
        }
	syscall = (void *)((long) syscall_node + sizeof(node_t));
	strncpy(syscall->name, name, MAX_NAME_LENGTH - 1);
	syscall->fptr = fptr;
	syscall->syscall_num = syscall_num;
	syscall->module = module;
	
        syscall_node->data = syscall;
		
	spin_lock(&syscall_tbl->lock);

	if(get_syscall(name)) {
		printk("Module already loaded\n");
                ret = -EINVAL;
		spin_unlock(&syscall_tbl->lock);
                goto err;
	}	

	add_node(syscall_node, &syscall_tbl->syscall_list, cmp_syscall_by_name);
	try_module_get(SYSCALL_MODULE);	
	spin_unlock(&syscall_tbl->lock);	
	printk("Syscall registered\n");
	goto out;
err:
	if(syscall_node) kfree(syscall_node);
out:
	return ret;
}

EXPORT_SYMBOL(register_syscall);

int unregister_syscall(char * name) {
	int ret = 0;
	node_t * syscall_node = NULL;
	
	spin_lock(&syscall_tbl->lock);	
	syscall_node = get_syscall(name);
	if(!syscall_node) {
		printk("Syscall does not exist\n");
		ret = -ENOENT;
		goto err;
	}

	if(module_refcount(XNODE_DATA(syscall_node, syscall_t *)->module) > 1) {
		printk("Syscall currently being used\n");
		ret = -EBUSY;
		goto err;
	}
	
	remove_node(syscall_node);
	kfree(syscall_node);
	module_put(SYSCALL_MODULE);	
	spin_unlock(&syscall_tbl->lock);	
	
	printk("Syscall unregistered\n");
	goto out;
err:
	spin_unlock(&syscall_tbl->lock);
out:
	return ret;
}

EXPORT_SYMBOL(unregister_syscall);

int add_syscall_vector() {
	int ret = 0;
	node_t * syscall_vector_node = NULL;
	syscall_vector_t * syscall_vector = NULL;

	syscall_vector_node = kmalloc(sizeof(node_t) + sizeof(syscall_vector_t), GFP_KERNEL);
        if(!syscall_vector_node) {
                printk("Memory initializtion failed\n");
                ret = -ENOMEM;
                goto err;
        }	

	syscall_vector = (void *)((long)syscall_vector_node + sizeof(node_t));
 
	syscall_vector->refcnt = 0;
	syscall_vector->id = 0;
        INIT_LIST_HEAD(XNODE_LIST(&syscall_vector->syscall_num_list));	
	
	syscall_vector_node->data = syscall_vector;
	memset(syscall_vector->blocked, 0, NUM_SYSCALLS);

	spin_lock(&syscall_tbl->lock);
	__add_syscall_vector(syscall_vector_node);
	ret = XNODE_DATA(syscall_vector_node, syscall_vector_t *)->id;
	try_module_get(SYSCALL_MODULE);	
	spin_unlock(&syscall_tbl->lock);
	printk("vector registered\n");
	goto out;

err:
	if(syscall_vector_node) kfree(syscall_vector_node);
out:
	return ret;
}

EXPORT_SYMBOL(add_syscall_vector);

int remove_syscall_vector(short vector_id) {
	int ret = 0;
	node_t * syscall_vector_node = NULL;
	syscall_vector_t * syscall_vector = NULL;

	spin_lock(&syscall_tbl->lock);
	
	syscall_vector_node = get_syscall_vector(vector_id);
	if(!syscall_vector_node) {
		printk("Syscall vector does not exist\n");
		ret = -ENOENT;
		goto err;		
	}	
	syscall_vector = XNODE_DATA(syscall_vector_node, syscall_vector_t *);
	
	if(syscall_vector->refcnt > 0) {
		printk("Syscall vector in use\n");
		ret = -EBUSY;
		goto err;		
	}

	remove_node(syscall_vector_node);
	module_put(SYSCALL_MODULE);	
	spin_unlock(&syscall_tbl->lock);

	printk("vector unregistered\n");
	kfree(syscall_vector_node);
	goto out;
err:
	spin_unlock(&syscall_tbl->lock);
out:
	return ret;
}

EXPORT_SYMBOL(remove_syscall_vector);

int add_syscall_to_vector(short vector_id, char * name) {
	node_t * syscall_node, * syscall_vector_node, * syscall_num_node;	
	int ret = 0;

	syscall_num_node = kmalloc(sizeof(node_t), GFP_KERNEL);
        if(!syscall_num_node) {
                printk("Memory initializtion failed\n");
                ret = -ENOMEM;
                goto mem_err;
        }

	spin_lock(&syscall_tbl->lock);
	syscall_node = get_syscall(name);
	if(!syscall_node) {
		printk("Syscall not found\n");
		ret = -ENOENT;
		goto err;
	}

	syscall_vector_node = get_syscall_vector(vector_id);
	if(!syscall_vector_node) {
                printk("Syscall vector not found\n");
                ret = -ENOENT;
                goto err;
        }

	if(get_syscall_of_syscall_vector(
                &XNODE_DATA(syscall_vector_node, syscall_vector_t *)->syscall_num_list, 
		XNODE_DATA(syscall_node, syscall_t *)->syscall_num) != NULL) {
		printk("Syscall num already present in the vector\n");
		ret = -1;
		goto err;
	}
 
	syscall_num_node->data = XNODE_DATA(syscall_node, syscall_t *);
	
	try_module_get(XNODE_DATA(syscall_node, syscall_t *)->module);
	add_node(syscall_num_node, &XNODE_DATA(syscall_vector_node, syscall_vector_t *)->syscall_num_list, cmp_syscall_by_syscall_num);
	spin_unlock(&syscall_tbl->lock);	

	printk("Vector id %d mapped to syscall %s\n", vector_id, name);
	goto out;

err:	
	spin_unlock(&syscall_tbl->lock);	
mem_err:
	if(syscall_num_node) kfree(syscall_num_node);
out:
	return ret;
}

EXPORT_SYMBOL(add_syscall_to_vector);

int remove_syscall_from_vector(short vector_id, char * name) {
	node_t * syscall_node, * syscall_vector_node, * syscall_num_node;
	int ret = 0;
        
	spin_lock(&syscall_tbl->lock); 
	
	syscall_node = get_syscall(name);
        if(!syscall_node) {
                printk("Syscall not found\n");
                ret = -ENOENT;
                goto err;
        }

        syscall_vector_node = get_syscall_vector(vector_id);
        if(!syscall_vector_node) {
                printk("Syscall vector not found\n");
                ret = -ENOENT;
                goto err;
        }

	syscall_num_node = get_syscall_of_syscall_vector(
		&XNODE_DATA(syscall_vector_node, syscall_vector_t *)->syscall_num_list, 
		XNODE_DATA(syscall_node, syscall_t *)->syscall_num); 
	if(!syscall_num_node) {
		printk("Syscall vector not found\n");
                ret = -ENOENT;
                goto err;
	}
	
	module_put(XNODE_DATA(syscall_node, syscall_t *)->module);
	remove_node(syscall_num_node);
	spin_unlock(&syscall_tbl->lock);
	kfree(syscall_num_node);
	printk("Vector id %d unmapped from syscall %s\n", vector_id, name);
	goto out;
err:
	spin_unlock(&syscall_tbl->lock); 
out:
	return ret;
}

EXPORT_SYMBOL(remove_syscall_from_vector);

int show_syscall_tbl(void) {
	struct list_head * pos, *pos1;
	syscall_vector_t * syscall_vector;
	syscall_t * syscall;
	printk("\nSyscall Vectors ");
	list_for_each(pos, XNODE_LIST(&syscall_tbl->vector_list)) {
		syscall_vector = XNODE_DATA(list_entry(pos, node_t, entry), syscall_vector_t *);
		printk("%d ", syscall_vector->id);
	}
	printk("\n");

	printk("Syscalls ");
	list_for_each(pos, XNODE_LIST(&syscall_tbl->syscall_list)) {
		syscall = XNODE_DATA(list_entry(pos, node_t, entry), syscall_t *);
                printk("%s ", syscall->name);
        }
	printk("\n");

	printk("Syscall vector - syscall mapping : ");
        list_for_each(pos, XNODE_LIST(&syscall_tbl->vector_list)) {
                syscall_vector = XNODE_DATA(list_entry(pos, node_t, entry), syscall_vector_t *);
                printk("Syscall Vector : %d - ", syscall_vector->id);
		list_for_each(pos1, XNODE_LIST(&syscall_vector->syscall_num_list)) {
                	syscall = XNODE_DATA(list_entry(pos1, node_t, entry), syscall_t *);
                	printk("%s ", syscall->name);
        	}
        }
        printk("\n");

	return 0;
}

EXPORT_SYMBOL(show_syscall_tbl);

int show_syscall_tbl_test(syscall_tbl_info_t * syscall_info) {
	int i = 0, j = 0;
	for(i = 0; i < syscall_info->size; ++i) {
                printk("Syscall Vector : %d - ", syscall_info->syscall_vector_info[i].vector_id);
		for(j = 0; j < syscall_info->syscall_vector_info[i].size; ++j) {
                	printk("%s ", syscall_info->syscall_vector_info[i].syscalls[j]);
		}
		printk("\n");
	}
	return 0;
}

EXPORT_SYMBOL(show_syscall_tbl_test);

int get_syscall_tbl_info(syscall_tbl_info_t * syscall_info) {
	struct list_head * pos, *pos1;
        syscall_vector_t * syscall_vector;
        syscall_t * syscall;
	int i=0, j=0;
        list_for_each(pos, XNODE_LIST(&syscall_tbl->vector_list)) {
                syscall_vector = XNODE_DATA(list_entry(pos, node_t, entry), syscall_vector_t *);
                syscall_info->syscall_vector_info[i].vector_id = syscall_vector->id;
		j = 0;
		list_for_each(pos1, XNODE_LIST(&syscall_vector->syscall_num_list)) {
                        syscall = XNODE_DATA(list_entry(pos1, node_t, entry), syscall_t *);
                        strcpy(syscall_info->syscall_vector_info[i].syscalls[j], syscall->name);
			j++;
                }
		syscall_info->syscall_vector_info[i].size = j;
		i++;
        }
	syscall_info->size = i;
        return 0;
}

EXPORT_SYMBOL(get_syscall_tbl_info);

static struct task_struct * get_process_by_pid(pid_t pid) {
	struct task_struct *task = NULL;
	for_each_process(task) {
		if(pid == task->pid) {
			return task;
		}
	}
	return NULL;
}

int get_vector_by_pid(pid_t pid) {
	int ret = 0;	
	struct task_struct *task = get_process_by_pid(pid);
	if(task == NULL) {
		ret = -EINVAL;
		printk("Unkown pid : %d\n", pid);
		goto out;
	}	
	ret = task->vector_id;
out:
	return ret;
}

EXPORT_SYMBOL(get_vector_by_pid);

int assign_syscall_vector(short vector_id, pid_t pid) {
	int ret = 0;
	node_t * syscall_vector_node;
	struct task_struct *task;
	task = get_process_by_pid(pid);
	if(task == NULL) {
		ret = -EINVAL;
		printk("Unkown pid : %d\n", pid);
		goto out;
	}

	if(vector_id == task->vector_id) {
		printk("Already assigned to vector id : %d", task->vector_id);
		ret = 0;
		goto out;
	}

	spin_lock(&syscall_tbl->lock);
	syscall_vector_node = get_syscall_vector(vector_id);
	if(!syscall_vector_node) {
		ret = -ENOENT;
		spin_unlock(&syscall_tbl->lock);
		printk("Syscall vector node does not exist\n");
		goto out;
	}
	task->vector_id = vector_id;
	XNODE_DATA(syscall_vector_node, syscall_vector_t *)->refcnt += 1;
	spin_unlock(&syscall_tbl->lock);
	printk("Incrementing refcount for vector id %d\n", vector_id);
out:
	return ret;
}

EXPORT_SYMBOL(assign_syscall_vector);

int unassign_syscall_vector(pid_t pid) {
	int ret = 0;
        node_t * syscall_vector_node;
	struct task_struct *task;
       	short vector_id = 0;;
	 
	task = get_process_by_pid(pid);
        if(task == NULL) {
                ret = -EINVAL;
                printk("Unkown pid : %d\n", pid);
                goto out;
        }
	
	vector_id = task->vector_id;
	
	if(vector_id == 0) {
		printk("Already set on default\n");
		ret = 0L;
		goto out;
	}
	
        spin_lock(&syscall_tbl->lock);
        syscall_vector_node = get_syscall_vector(vector_id);
        if(!syscall_vector_node) {
                ret = -ENOENT;
		printk("Syscall vector node does not exist\n");
                goto err;
        }
	if(XNODE_DATA(syscall_vector_node, syscall_vector_t *)->refcnt == 0) {
		ret = -1;
		printk("Syscall vector is already 0\n");
                goto err;
	}
	task->vector_id = 0;
	XNODE_DATA(syscall_vector_node, syscall_vector_t *)->refcnt -= 1;
	spin_unlock(&syscall_tbl->lock);
	printk("Decrementing refcount for vector id %d\n", vector_id);
	goto out;
err:	
	spin_unlock(&syscall_tbl->lock);
out:
        return ret;
}

EXPORT_SYMBOL(unassign_syscall_vector);


int reassign_syscall_vector(short vector_id, pid_t pid) {
        
	int ret = 0;
        node_t * syscall_vector_node = NULL;
        node_t * old_syscall_vector_node = NULL;
        struct task_struct *task = NULL;
	short old_vector_id = 0;

	task = get_process_by_pid(pid);

        if(task == NULL) {
                ret = -EINVAL;
                printk("Unkown pid : %d\n", pid);
                goto out;
        }
	
	old_vector_id = task->vector_id;
	
	// same vector ids
	if(vector_id == old_vector_id) {
		ret = 0;
		goto out;
	}
	
	// unassigning vector id to default
	if(old_vector_id != 0 && vector_id == 0) {
		printk("Redirect to unassign\n");
		ret =  unassign_syscall_vector(pid);
		goto out;
	}
	
	// assigning from default to vector id
	if(old_vector_id == 0 && vector_id != 0) {
		printk("Redirect to assign\n");
		ret = assign_syscall_vector(vector_id, pid);
		goto out;
	}

        task = get_process_by_pid(pid);
        
	if(task == NULL) {
                ret = -EINVAL;
                printk("Unkown pid : %d\n", pid);
                goto out;
        }

        spin_lock(&syscall_tbl->lock);
        
	syscall_vector_node = get_syscall_vector(vector_id);
        if(!syscall_vector_node) {
                ret = -ENOENT;
                printk("Syscall vector node does not exist\n");
                goto err;
        }

	old_syscall_vector_node = get_syscall_vector(old_vector_id);
	if(!old_syscall_vector_node) {
		ret = -ENOENT;
                printk("Old syscall vector node does not exist\n");
                goto err;
        }
       
	if(XNODE_DATA(old_syscall_vector_node, syscall_vector_t *)->refcnt == 0) {
        	ret = -1;
                printk("Ref count of old syscall vector is already 0\n");
                goto err;
        }
 
	XNODE_DATA(old_syscall_vector_node, syscall_vector_t *)->refcnt -= 1;
        XNODE_DATA(syscall_vector_node, syscall_vector_t *)->refcnt += 1;
	
	task->vector_id = vector_id;
	
        spin_unlock(&syscall_tbl->lock);
        
	printk("Decrementing refcount for vector id %d\n", old_vector_id);
        printk("Incrementing refcount for vector id %d\n", vector_id);
        goto out;
err:
        spin_unlock(&syscall_tbl->lock);
out:
        return ret;
}

EXPORT_SYMBOL(reassign_syscall_vector);

int block_syscall_num(short vector_id, short syscall_num) {
	node_t * syscall_vector_node = NULL;
	int ret = 0;
	spin_lock(&syscall_tbl->lock);	
	syscall_vector_node = get_syscall_vector(vector_id);
        if(!syscall_vector_node) {
                printk("Syscall vector not found\n");
                ret = -ENOENT;
                goto out;
        }
	XNODE_DATA(syscall_vector_node, syscall_vector_t *)->blocked[syscall_num / 8] |= (1 << (syscall_num % 8));
	spin_unlock(&syscall_tbl->lock);
out:
	return ret;	
}

EXPORT_SYMBOL(block_syscall_num);

int unblock_syscall_num(short vector_id, short syscall_num) {
	node_t * syscall_vector_node = NULL;
	int ret = 0;
	spin_lock(&syscall_tbl->lock);
        syscall_vector_node = get_syscall_vector(vector_id);
        if(!syscall_vector_node) {
                printk("Syscall vector not found\n");
                ret = -ENOENT;
                goto out;
        }
	XNODE_DATA(syscall_vector_node, syscall_vector_t *)->blocked[syscall_num / 8] &= ~(1 << (syscall_num % 8));
        spin_unlock(&syscall_tbl->lock);
out:
	return ret;
}

EXPORT_SYMBOL(unblock_syscall_num);

unsigned long get_syscall_fn(short vector_id, short syscall_num) {
	
	unsigned long ret = 0;
        node_t * syscall_vector_node = NULL, * syscall_num_node = NULL;
	
	spin_lock(&syscall_tbl->lock);
        
	syscall_vector_node = get_syscall_vector(vector_id);
        if(!syscall_vector_node) {
                // printk("Syscall vector not found\n");
                ret = -ENOENT;
                goto err;
        }
	
	if(is_syscall_blocked(syscall_num, XNODE_DATA(syscall_vector_node, syscall_vector_t *)->blocked)) {
		printk("Syscall is blocked\n");
		ret = -ENOSYS;
		goto err;
	}

        syscall_num_node =  get_syscall_of_syscall_vector(
                &XNODE_DATA(syscall_vector_node, syscall_vector_t *)->syscall_num_list, syscall_num);
        if(!syscall_num_node) {
                printk("Syscall number in the vector not found\n");
                ret = 0;
                goto err;
        }

        ret = XNODE_DATA(syscall_num_node, syscall_t *)->fptr;
	spin_unlock(&syscall_tbl->lock);
	goto out;
err:
	spin_unlock(&syscall_tbl->lock);
out:
        return ret;
}

EXPORT_SYMBOL(get_syscall_fn);

MODULE_AUTHOR("Group 1*");
MODULE_DESCRIPTION("Per process systemm call table");
MODULE_LICENSE("GPL");

module_init(init_syscall_tbl);
module_exit(exit_syscall_tbl);
