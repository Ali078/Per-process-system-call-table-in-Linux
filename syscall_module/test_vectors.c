#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>
#include "syscall_tbl.h"




extern int show_syscall_tbl(void);
extern int add_syscall_vector(void);
extern int remove_syscall_vector(short vector_id);
extern int add_syscall_to_vector(short vector_id, char * name);
extern int remove_syscall_from_vector(short vector_id, char * name);

int vector_id1 = 0;
int vector_id2 = 0;
static int __init init_test_vectors(void)
{
        int status = 0;
        vector_id1 = add_syscall_vector();
        add_syscall_to_vector(vector_id1,"read2");
        add_syscall_to_vector(vector_id1,"open2");
        add_syscall_to_vector(vector_id1,"close2");
        add_syscall_to_vector(vector_id1,"rename2");
        add_syscall_to_vector(vector_id1,"rmdir2");

        vector_id2 = add_syscall_vector();
        add_syscall_to_vector(vector_id2,"mkdir2");
        add_syscall_to_vector(vector_id2,"link2");
        add_syscall_to_vector(vector_id2,"unlink2");
        add_syscall_to_vector(vector_id2,"chmod2");
        status = show_syscall_tbl(); 
         
	printk("REGISTERED SYSTEM VECTORS \n");
	
	return 0;
}


static void __exit exit_test_vectors(void)
{
       int status = 0;
       status = remove_syscall_from_vector(vector_id1,"read2");
       status = remove_syscall_from_vector(vector_id1,"open2");
       status = remove_syscall_from_vector(vector_id1,"close2");
       status = remove_syscall_from_vector(vector_id1,"rename2");
       status = remove_syscall_from_vector(vector_id1,"rmdir2");

       status = remove_syscall_from_vector(vector_id2,"mkdir2");
       status = remove_syscall_from_vector(vector_id2,"link2");
       status = remove_syscall_from_vector(vector_id2,"unlink2");
       status = remove_syscall_from_vector(vector_id2,"chmod2");
       status = remove_syscall_vector(vector_id1);
       status = remove_syscall_vector(vector_id2);
       vector_id1=0;
       vector_id2=0;
             
       printk("REMOVED SYSTEM VECTORS\n");

}
MODULE_AUTHOR("Group 1*");
MODULE_DESCRIPTION(" test vector module");
MODULE_LICENSE("GPL");

module_init(init_test_vectors);
module_exit(exit_test_vectors);

