#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <asm/syscall_tbl_api.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/linkage.h>
#include <linux/moduleloader.h>
#include <linux/buffer_head.h>
#include <linux/limits.h>

#include "hw3_ioctl.h"
#include "syscall_tbl.h"

#define DEVICE_NAME "hw3_ioctl_test_device"


extern int register_syscall(char * name, unsigned long fptr, short syscall_num, struct module * module);
extern int unregister_syscall(char * name);
extern int show_syscall_tbl(void);
extern int add_syscall_vector(void);
extern int remove_syscall_vector(short vector_id);
extern int add_syscall_to_vector(short vector_id, char * name);
extern int remove_syscall_from_vector(short vector_id, char * name);
extern node_t * get_syscall(char * name);
extern node_t * get_syscall_vector(short id);
extern node_t * get_syscall_of_syscall_vector(node_t * syscall_num_list, short syscall_num);
extern unsigned long get_syscall_fn(short syscall_num, short vector_id);
extern int assign_syscall_vector(short vector_id, pid_t pid);
extern int unassign_syscall_vector(pid_t pid);
extern int reassign_syscall_vector(short vector_id, pid_t pid);
extern int get_vector_by_pid(pid_t pid);
extern int get_syscall_tbl_info(syscall_tbl_info_t *syscall_info);
extern int show_syscall_tbl_test(syscall_tbl_info_t * syscall_info); 
extern int block_syscall_num(short vector_id, short syscall_num);
extern int unblock_syscall_num(short vector_id, short syscall_num);

struct myargs {
	short vector_id;
	short syscall_num;
	char *name;
	pid_t pid;
	syscall_tbl_info_t *sys;
} typedef myargs;

static long manipulate_syscalls(struct file *file, unsigned int cmd,
				unsigned long arg) 
{
	int rc = 0;
	
	myargs *arglist;
	myargs *src;
	short s_rc;

	src = (myargs *)arg;

	s_rc = 0;
	arglist = kmalloc(sizeof(struct myargs), GFP_KERNEL);
	
	if (!arglist) {
		rc = -ENOMEM;
		goto out_arglist;
	}

	arglist->name = NULL;
	arglist->vector_id = 0;
	arglist->syscall_num = 0;
	arglist->pid = 0;
	arglist->sys = NULL;

	if (src->name != NULL) {
		arglist->name = kmalloc(strlen(src->name)+1, GFP_KERNEL);
		if (!arglist->name) {
			rc = -ENOMEM;
			goto out_name;
		}
		rc = copy_from_user(arglist->name, src->name, strlen(src->name));
		if (rc) {
			rc = (-1 * rc);
			goto out_name;
		}
		arglist->name[strlen(src->name)] = '\0';
	}
	if (src->vector_id) {
		rc = copy_from_user(&arglist->vector_id, &src->vector_id, 
				sizeof(short));
		if (rc) {
			rc = (-1 * rc);
			goto out_name;
		}
	}
	if (src->syscall_num) {
                rc = copy_from_user(&arglist->syscall_num, &src->syscall_num,
                                sizeof(short));
                if (rc) {
                        rc = (-1 * rc);
                        goto out_name;
                }
        }
	if (src->pid) {
		rc = copy_from_user(&arglist->pid, &src->pid, 
				sizeof(pid_t));
		if (rc) {
			rc = (-1 * rc);
			goto out_name;
		}
	}

	arglist->sys = kmalloc(sizeof(syscall_tbl_info_t), GFP_KERNEL);
	if (!arglist->sys) {
		printk("couldn't allocate mem for arglist->sys\n");
		goto out_sys;
	}
	
	switch (cmd) {
	case IOCTL_SYSCALL_TBL_ADD_VECTOR:
			rc = add_syscall_vector();
			printk(" if successful. then rc = %d\n", rc);
			if (rc < 0 ) {
				printk("Return value < 0. Some error\n");
				goto out_name;
			}
			break;

	case IOCTL_SYSCALL_TBL_REMOVE_VECTOR:
			rc = remove_syscall_vector(arglist->vector_id);
			printk(" if successful. then rc = %d\n", rc);
			if (rc < 0) {
				printk(" Return value < 0. Couldn't remove syscall\n");
				goto out_name;
			}
			break;
	case IOCTL_SYSCALL_TBL_ADD_SYSCALL_TO_VECTOR:
			rc = add_syscall_to_vector(arglist->vector_id,
						   arglist->name);
			printk("if successful. then rc = %d\n", rc);
			if (rc < 0) {
				printk("Couldn't add syscall to vector\n");
				goto out_name;
			}		
			break;

	case IOCTL_SYSCALL_TBL_REMOVE_SYSCALL_FROM_VECTOR:
			rc = remove_syscall_from_vector(arglist->vector_id,
							arglist->name);
			printk(" if successful. then rc = %d\n", rc);
			if (rc < 0) {
				printk(" Couldn't remove from syscall\n");
				goto out_name;
			}
			break;
	case IOCTL_SYSCALL_TBL_ASSIGN_VECTOR_TO_PROCESS:
			rc = reassign_syscall_vector(arglist->vector_id,
						   arglist->pid);
			printk(" if successful. then rc = %d\n", rc);
			if (rc < 0) {
				printk("Couldn't assign syscall to vector\n");
				goto out_name;
			}
			break;
	case IOCTL_SYSCALL_TBL_GET_VECTOR_ID_OF_PROCESS:
			rc = get_vector_by_pid(arglist->pid);
			printk("vector id of given ps = %d\n", rc);
			if (rc < 0) {
				printk("Couldn't get vector by pid\n");
				goto out_name;
			}
			s_rc = rc;
			rc = copy_to_user(&(src->vector_id), &s_rc,
					  sizeof(short));
			printk("rc value here = %d\n", rc);
			if (rc < 0)
				printk("some error in sending value back to user\n");
			break;
	case IOCTL_SYSCALL_TBL_GET_INFO:
			rc = get_syscall_tbl_info(arglist->sys);
			if (rc < 0) {
				printk("some error in get_syscall_tbl_info\n");
			}
			show_syscall_tbl_test(arglist->sys);
			rc = copy_to_user(src->sys, arglist->sys, sizeof(syscall_tbl_info_t));
			printk("value of rc = %d\n",rc);
			if (rc < 0) {
				printk("some error in copy to user\n");
			}
			printk("still in ioctl. before brea\n");
			break;
	case IOCTL_SYSCALL_TBL_BLOCK:
			rc = block_syscall_num(arglist->vector_id, arglist->syscall_num);	
			break;
	case IOCTL_SYSCALL_TBL_UNBLOCK:
			rc = unblock_syscall_num(arglist->vector_id, arglist->syscall_num);	
			break;
	default:
		break;
	}
out_sys:
	if(arglist->sys)
		kfree(arglist->sys);
out_name:
	if (arglist->name)
		kfree(arglist->name);

out_arglist:
	if (arglist)
		kfree(arglist);

	return rc;
}


const struct file_operations ioctl_fops = {

	.unlocked_ioctl = manipulate_syscalls,

};

static int __init init_ioctl_module(void)
{
	int rc = 0;
	printk("Ioctl module successfully loaded\n");
	rc = register_chrdev(MAJOR_NUM , DEVICE_NAME, &ioctl_fops);
	if (rc < 0) {
		printk("Some error in registering tbl_vector_magic\n");
		goto out;
	}

out:
	return rc;
}

static void __exit exit_ioctl_module(void)
{
	/*
	 * unregister_chrdev returns void.
	 * no need to check to ensure if successful
	 */
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);

}



MODULE_LICENSE("GPL");
MODULE_AUTHOR("CSE506P14");
MODULE_DESCRIPTION("IOCTL module to enable users to change syscall vector"
		   "used by a running process.");


module_init(init_ioctl_module);
module_exit(exit_ioctl_module);
