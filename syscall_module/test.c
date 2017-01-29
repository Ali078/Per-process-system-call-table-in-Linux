#include "syscall_tbl.h"
#include <linux/slab.h>

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
extern int block_syscall_num(short vector_id, short syscall_num);
extern int unblock_syscall_num(short vector_id, short syscall_num);
extern int get_syscall_tbl_info(syscall_tbl_info_t * syscall_info);
extern int show_syscall_tbl_test(syscall_tbl_info_t * syscall_info);

#define TEST_CASE(cond) if(!(cond)) { printk("TEST CASE FAILURE - TC: %d\n", __LINE__); return;}

void test_register_unregister1(void) {
	int i = 0;
	printk("\nstart test_register_unregister1\n");

	
	register_syscall("open", 1, 1, THIS_MODULE);
	unregister_syscall("open");	
	TEST_CASE(get_syscall("open") == NULL);


	register_syscall("open", 1, 1, THIS_MODULE);
	i = register_syscall("open", 1, 1, THIS_MODULE);
	TEST_CASE(i<0);
	unregister_syscall("open");
	
	i = unregister_syscall("open");
	TEST_CASE(i<0);


	register_syscall("open", 5, 1, THIS_MODULE);
	register_syscall("close", 2, 1, THIS_MODULE);
	register_syscall("read", 27, 1, THIS_MODULE);
	register_syscall("write", 1, 1, THIS_MODULE);
	unregister_syscall("read");
	register_syscall("read", 27, 1, THIS_MODULE);
	unregister_syscall("read");
	unregister_syscall("open");
	unregister_syscall("close");
	unregister_syscall("write");
	
	i = unregister_syscall("read");
	TEST_CASE(i<0);
	i = unregister_syscall("open");
	TEST_CASE(i<0);
	i = unregister_syscall("close");
	TEST_CASE(i<0);
	i = unregister_syscall("write");
	TEST_CASE(i<0);
	

	printk("\nend test_register_unregister1\n");
}

void test_add_remove_syscall_vector1(void) {
	int id = 0, id1 = 0, id2 = 0, id3 = 0;
        printk("\nstart test_add_remove_syscallvector1\n");
	
	id = add_syscall_vector();
	remove_syscall_vector(id);
	TEST_CASE(get_syscall_vector(id) == NULL);	

	id1 = add_syscall_vector();
	id2 = add_syscall_vector();
	id3 = add_syscall_vector();
	remove_syscall_vector(id2);
	remove_syscall_vector(id1);
	id2 = add_syscall_vector();
	remove_syscall_vector(id2);
	id1 = add_syscall_vector();
	id2 = add_syscall_vector();
	remove_syscall_vector(id3);
	remove_syscall_vector(id2);
	remove_syscall_vector(id1);

	TEST_CASE(get_syscall_vector(id1) == NULL);
	TEST_CASE(get_syscall_vector(id2) == NULL);
	TEST_CASE(get_syscall_vector(id3) == NULL);

	printk("\nend test_add_remove_syscallvector1\n");
}

void test_assign_unassign_syscall_vector1(void) {
	int id1, id2;
	printk("\nstart test_assign_unassign_syscall_vector1\n");
	id1 = add_syscall_vector();

	assign_syscall_vector(id1, 1);
	TEST_CASE(remove_syscall_vector(id1) < 0);	
	unassign_syscall_vector(1);
	TEST_CASE(remove_syscall_vector(id1) == 0);	
	
	id1 = add_syscall_vector();
	assign_syscall_vector(id1, 1);
	TEST_CASE(assign_syscall_vector(id1, 1) == 0);
	TEST_CASE(get_vector_by_pid(1) == id1);
	assign_syscall_vector(id1, 2);
	assign_syscall_vector(id1, 3);
	unassign_syscall_vector(1);
	TEST_CASE(get_vector_by_pid(1) == 0);
	assign_syscall_vector(id1, 1);
	unassign_syscall_vector(1);	
	unassign_syscall_vector(2);	
	unassign_syscall_vector(3);	
	TEST_CASE(unassign_syscall_vector(1) == 0);
	assign_syscall_vector(id1, 1);	
	unassign_syscall_vector(id1);	
	remove_syscall_vector(id1);
	
	id1 = add_syscall_vector();
	id2 = add_syscall_vector();
	assign_syscall_vector(id1, 1);
	reassign_syscall_vector(id2, 1);
	TEST_CASE(get_vector_by_pid(1) == id2);
	TEST_CASE(remove_syscall_vector(id1) == 0);
	TEST_CASE(remove_syscall_vector(id2) < 0);
	TEST_CASE(get_syscall_vector(id2) != NULL);
	reassign_syscall_vector(0, 1);
	TEST_CASE(remove_syscall_vector(id2) == 0);

	id1 = add_syscall_vector();
	id2 = add_syscall_vector();
	assign_syscall_vector(id1, 1);
	reassign_syscall_vector(id2, 1);
	reassign_syscall_vector(id1, 1);
	TEST_CASE(unassign_syscall_vector(1) == 0);
	TEST_CASE(remove_syscall_vector(id1) == 0);
	TEST_CASE(remove_syscall_vector(id2) == 0);
	
	id1 = add_syscall_vector();
	reassign_syscall_vector(id1, 1);
	TEST_CASE(remove_syscall_vector(id1) < 0);
	reassign_syscall_vector(0, 1);
	TEST_CASE(remove_syscall_vector(id1) == 0);
        	

	printk("\nend test_assign_unassign_syscall_vector1\n");

		
}

void test_add_remove_syscall_mapping1(void) {
	int id1 = 0, id2 = 0;
	long test = 0;

	printk("\nstart test_add_remove_syscall_mapping1\n");
	
	register_syscall("open", 1, 1, THIS_MODULE);
        id1 = add_syscall_vector();
	add_syscall_to_vector(id1, "open");
	test = get_syscall_fn(1, id1);
	TEST_CASE(test > 0 && test != -ENOENT);	
	remove_syscall_from_vector(id1, "open"); 
	unregister_syscall("open");
	remove_syscall_vector(id1);	


	register_syscall("open", 1, 1, THIS_MODULE);
	register_syscall("close", 2, 2, THIS_MODULE);
        id1 = add_syscall_vector();
        id2 = add_syscall_vector();

	add_syscall_to_vector(id1, "open");
	TEST_CASE(add_syscall_to_vector(id1, "open") < 0);
	add_syscall_to_vector(id1, "close");
	
	remove_syscall_from_vector(id1, "close");
	TEST_CASE(remove_syscall_from_vector(id1, "close") < 0);
	remove_syscall_from_vector(id1, "open");	
	
	unregister_syscall("open");
        remove_syscall_vector(id1);
	unregister_syscall("close");
        remove_syscall_vector(id2);


	register_syscall("open", 1, 1, THIS_MODULE);
        register_syscall("close", 2, 2, THIS_MODULE);
        id1 = add_syscall_vector();
        id2 = add_syscall_vector();
	add_syscall_to_vector(id1, "open");
	add_syscall_to_vector(id1, "close");
	
	add_syscall_to_vector(id2, "open");
	add_syscall_to_vector(id2, "close");
	
	remove_syscall_from_vector(id1, "close");
	remove_syscall_from_vector(id2, "open");	
	TEST_CASE(remove_syscall_from_vector(id1, "close") < 0);
	TEST_CASE(remove_syscall_from_vector(id2, "open") < 0);

	remove_syscall_from_vector(id1, "open");

	TEST_CASE(add_syscall_to_vector(id2, "close") < 0);
        add_syscall_to_vector(id2, "open");

	remove_syscall_from_vector(id2, "open");
	remove_syscall_from_vector(id2, "close");	
	
	unregister_syscall("open");
        remove_syscall_vector(id1);
        unregister_syscall("close");
        remove_syscall_vector(id2);
	
	printk("\nend test_add_remove_syscall_mapping1\n");
}

void test_get_syscall_fn1(void) {
	int id1;
	id1 = add_syscall_vector();
	register_syscall("open", 1, 2, THIS_MODULE);
	register_syscall("close", 4, 3, THIS_MODULE);
        add_syscall_to_vector(id1, "open");

	show_syscall_tbl();

	TEST_CASE(get_syscall_fn(id1, 2) == 1);
	
	remove_syscall_from_vector(id1, "open");
	
	unregister_syscall("open");
        remove_syscall_vector(id1);	

	id1 = add_syscall_vector();
        register_syscall("open", 1, 2, THIS_MODULE);
        add_syscall_to_vector(id1, "open");
        add_syscall_to_vector(id1, "close");

        TEST_CASE(get_syscall_fn(id1, 2) == 1);

	block_syscall_num(id1, 2);

	TEST_CASE(get_syscall_fn(id1, 3) == 4);
        
	TEST_CASE(get_syscall_fn(id1, 2) == -ENOSYS);

	block_syscall_num(id1, 3);
	
	unblock_syscall_num(id1, 2);

	TEST_CASE(get_syscall_fn(id1, 2) == 1);
	
	TEST_CASE(get_syscall_fn(id1, 3) == -ENOSYS);
	
	unblock_syscall_num(id1, 3);

	remove_syscall_from_vector(id1, "open");
	remove_syscall_from_vector(id1, "close");

        unregister_syscall("open");
        unregister_syscall("close");
        remove_syscall_vector(id1);

	id1 = add_syscall_vector();
        register_syscall("open", 1, 2, THIS_MODULE);
        register_syscall("close", 4, 3, THIS_MODULE);
        add_syscall_to_vector(id1, "open");
        add_syscall_to_vector(id1, "close");

        block_syscall_num(id1, 2);
        TEST_CASE(get_syscall_fn(id1, 2) == -ENOSYS);
        TEST_CASE(get_syscall_fn(id1, 3) == 4);
        block_syscall_num(id1, 3);
        TEST_CASE(block_syscall_num(id1, 3) == 0);
        TEST_CASE(get_syscall_fn(id1, 3) == -ENOSYS);
        unblock_syscall_num(id1, 2);
        TEST_CASE(get_syscall_fn(id1, 2) == 1);
        unblock_syscall_num(id1, 3);
        TEST_CASE(unblock_syscall_num(id1, 3) == 0);
        TEST_CASE(get_syscall_fn(id1, 3) == 4);

        remove_syscall_from_vector(id1, "open");
        remove_syscall_from_vector(id1, "close");

        unregister_syscall("open");
        unregister_syscall("close");
        remove_syscall_vector(id1);

	
}

void test_get_syscall_info_tbl1(void) {
	int id1, id2, id3;
	
	syscall_tbl_info_t *  info = kmalloc(sizeof(syscall_tbl_info_t), GFP_KERNEL);
	
	id1 = add_syscall_vector();
	id2 = add_syscall_vector();
	id3 = add_syscall_vector();
        register_syscall("open", 1, 2, THIS_MODULE);
        register_syscall("close", 4, 3, THIS_MODULE);
        add_syscall_to_vector(id1, "open");
        add_syscall_to_vector(id1, "close");
        add_syscall_to_vector(id2, "close");

	get_syscall_tbl_info(info);
	show_syscall_tbl_test(info);

	remove_syscall_from_vector(id1, "open");
        remove_syscall_from_vector(id1, "close");
        remove_syscall_from_vector(id2, "close");
	unregister_syscall("open");
        unregister_syscall("close");
        remove_syscall_vector(id1);
        remove_syscall_vector(id2);
        remove_syscall_vector(id3);

	kfree(info);
}

static int __init init_syscall_tbl(void)
{
	test_register_unregister1();
	test_add_remove_syscall_vector1();
	test_assign_unassign_syscall_vector1();
	test_add_remove_syscall_mapping1();
	test_get_syscall_fn1();
	test_get_syscall_info_tbl1();
	return 0;
}


static void __exit exit_syscall_tbl(void)
{	
}





MODULE_AUTHOR("Group 1*");
MODULE_DESCRIPTION("Per process systemm call table");
MODULE_LICENSE("GPL");

module_init(init_syscall_tbl);
module_exit(exit_syscall_tbl);
