#include <linux/linkage.h>
#include <linux/moduleloader.h>
#include <linux/sched.h>


extern int (*_assign_syscall_vector)(short, pid_t);

asmlinkage extern long (*sysptr)(unsigned long	clone_flags,
				unsigned long 	newsp,
				int __user 	*parent_tidptr,
				int __user 	*child_tidptr,
				unsigned long	tls_val,
				short int 	vector_id);

asmlinkage long clone2(unsigned long 	clone_flags,
			unsigned long	newsp,
			int __user	*parent_tidptr,
			int __user 	*child_tidptr,
			unsigned long 	tls_val,
			short int 	vector_id)
{
	long pid = 0;
	/*TODO: remove this print access user memory directly
	printk("flag = %ld newsp = %ld ptid = 0x%x\nctild = 0x%x tls_val = %ld  vector_id = %d\n"
		,clone_flags,newsp,*parent_tidptr,*child_tidptr,tls_val,vector_id);*/

	pid = _do_fork(clone_flags, newsp, 0, parent_tidptr, child_tidptr, tls_val);

	if (!(clone_flags & CLONE_SYSCALLS) && vector_id != 0 && _assign_syscall_vector) {
		_assign_syscall_vector(vector_id, pid);
	}

	printk("Pid retured by _do_fork %ld\n", pid);
	return pid;
}

static int __init init_sys_clone2(void)
{
	printk("installed new sys_clone2 module\n");
	if (sysptr == NULL)
		sysptr = clone2;
	return 0;
}

static void  __exit exit_sys_clone2(void)
{
	if (sysptr != NULL)
		sysptr = NULL;
	printk("removed sys_clone2 module\n");
}

module_init(init_sys_clone2);
module_exit(exit_sys_clone2);
MODULE_LICENSE("GPL");
