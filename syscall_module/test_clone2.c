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

#define STACK_SIZE (1024 * 1024)    /* Stack size for cloned child */

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                               } while (0)

#define BUF_SIZE 30
int fd = -1;

struct myargs {
	short vector_id;
	char *name;
	pid_t pid;
	syscall_tbl_info_t *sys;
} typedef myargs;


static int childVID1()
{	
	char ioctl_device_location[] = "/dev/hw3_ioctl_test_device";
	int rc = -1;
	myargs send_arg;
        char *buffer = NULL;
        int input_fd = -1;
        int bytes_read = 0;

        send_arg.vector_id = 0;
	send_arg.name = NULL;
	send_arg.pid = gettid();
	printf("id of child1 %d\n", send_arg.pid);	

	/*fd = open(ioctl_device_location, O_RDONLY);
        if (fd < 0) {
                 printf("Is hw3_ioctl_device module loaded?\n");
                 return -EACCES;
        }*/

	printf("Inside ChildVID1\n");
	rc = ioctl(fd, IOCTL_SYSCALL_TBL_GET_VECTOR_ID_OF_PROCESS, &send_arg);
        printf("Performing syscalls for Process ID = %d Vector ID = %d fd = %d rc = %d\n", send_arg.pid, send_arg.vector_id, fd, rc);
	sleep(2);   

	printf("\n Performing open,read,close on dtest_read.txt VID 1\n");

        buffer = malloc(100);

        input_fd = open("test_read.txt", O_RDONLY,0);
	printf(" return value for open1 = %d\n" ,input_fd);
	
	bytes_read = read(input_fd,buffer,BUF_SIZE);
	printf(" return value for read1 = %d\n" ,bytes_read);
	buffer[bytes_read] = '\0';
        printf("Content read: %s\n",buffer);

        rc = close(input_fd); 
	printf(" return value for close1 = %d\n" ,rc);

	free(buffer);

        sleep(3);

        printf("Performing rename, remove directory  VID 1 \n");

        rc = rename("abc1.protect","xyz1");
	printf(" return value for rename1 = %d\n" ,rc);


        rc = rmdir("def1.protect");	
	printf(" return value for rmdir1 = %d\n" ,rc);

	printf("\n\nTerminating now process for ChildVID1\n\n\n");
	exit(EXIT_SUCCESS);
	return 0;         
}


static int childVID2()
{
	char ioctl_device_location[] = "/dev/hw3_ioctl_test_device";
	int rc = -1;
	myargs send_arg;
        
	send_arg.vector_id = 0;
	send_arg.name = NULL;
	send_arg.pid = gettid();

	printf("id of child0 %d\n", send_arg.pid);

	/*fd = open(ioctl_device_location, O_RDONLY);
        if (fd < 0) {
                 printf("Is hw3_ioctl_device module loaded?\n");
                 return -EACCES;
        }*/

	printf("Inside ChildVID2\n");

	rc = ioctl(fd, IOCTL_SYSCALL_TBL_GET_VECTOR_ID_OF_PROCESS, &send_arg);
        printf("Performing syscalls for Process ID = %d Vector ID = %d fd =%d rc = %d\n", send_arg.pid, send_arg.vector_id, fd, rc); 
	sleep(2);

	printf("\n Performing link,unlink,chmod and mkdir operations VID 2\n");

        rc = link("default_file.txt","default_file_hlink2.txt"); 
	printf(" return value for link2 = %d\n" ,rc);

	rc = unlink("test_unlink2.txt");
	printf(" return value for unlink2 = %d\n" ,rc);
	
	rc = chmod("test_write2.txt",777);
	printf(" return value for chmod2 = %d\n" ,rc);
	
	rc = mkdir("new_dir2", 0700);
	printf(" return value for mkdir2 = %d\n" ,rc);

      	printf("\n\nTerminating now process for ChildVID2\n\n\n");
	exit(EXIT_SUCCESS);
	return 0;         
}


int main()
{
     	char *stack;                    /* Start of stack buffer */
       	char *stackTop;                 /* End of stack buffer */
       	int pid;
	short int vector_id;
	char ioctl_device_location[] = "/dev/hw3_ioctl_test_device";
	int rc = -1;
	myargs send_arg;
        char *buffer = NULL;
	int input_fd = -1; 
        int bytes_read = 0;
 
        send_arg.vector_id = 0;
	send_arg.name = NULL;
	send_arg.pid = gettid();
	printf("id of parent %d\n", send_arg.pid);
	
	fd = open(ioctl_device_location, O_RDONLY);
        if (fd < 0) {
                 printf("Is hw3_ioctl_device module loaded?\n");
                 return -EACCES;
        }
	
	printf("Inside Parent Process\n");
	
	rc = ioctl(fd, IOCTL_SYSCALL_TBL_GET_VECTOR_ID_OF_PROCESS, &send_arg);
        printf("Performing syscalls for Process ID = %d Vector ID = %d fd =%d rc = %d\n", send_arg.pid, send_arg.vector_id, fd, rc);

	printf("\nPerforming open,read,close on test_read.txt VID Default\n");

        buffer = malloc(100);

        input_fd = open("test_read.txt", O_RDONLY,0);
	printf(" return value for open = %d\n",input_fd);

        bytes_read = read(input_fd,buffer,BUF_SIZE);
	printf(" return value for read = %d\n", bytes_read);
	buffer[bytes_read] = '\0';
        printf("Content read: %s\n",buffer);

        rc = close(input_fd);	
	printf(" return value for close = %d\n" ,rc);

        free(buffer);
        
        sleep(3); 
       
        printf("\nPerforming rename and  rmdir using VID default\n");
         
        rc = rename("abc.protect","xyz"); 
	printf(" return value for rename = %d\n" ,rc);
	
	rc = rmdir("def.protect"); 
	printf(" return value for rmdir = %d\n" ,rc);
       
        printf("\n\n Performing link, unlink, chmod and mkdir operations\n");
       
	rc = link("default_file.txt","default_file_hlink.txt"); 
	printf(" return value for link = %d\n" ,rc);

	rc = unlink("test_unlink.txt");       
	printf(" return value for unlink = %d\n" ,rc);

	rc = chmod("test_write.txt",777);        
	printf(" return value for chmod = %d\n" ,rc);

	rc = mkdir("new_dir", 0700);
	printf(" return value for mkdir = %d\n" ,rc);

	
	printf("\n\nDone with Performing Syscalls for default syscall Table\n\n");

        /* Allocate stack for child */
        stack = malloc(STACK_SIZE);
       	if (stack == NULL)
        	errExit("malloc");
        stackTop = stack + STACK_SIZE; 

	vector_id = 0x01;
	pid = sys_clone2_wrapper(childVID1, stackTop, SIGCHLD, NULL , vector_id);
	
	if (pid == -1)
        	errExit("clone");
       	printf("First clone2() returned pid = %ld\n", (long) pid);

      	if (waitpid(pid, NULL, 0) == -1)    /* Wait for child */
     	      errExit("waitpid");
	
	
	
	 /* Allocate stack for child */
        stack = malloc(STACK_SIZE);
       	if (stack == NULL)
        	errExit("malloc");
        stackTop = stack + STACK_SIZE; 

	vector_id = 0x02;
	pid = sys_clone2_wrapper(childVID2, stackTop, SIGCHLD, NULL , vector_id);
	
	if (pid == -1)
        	errExit("clone");
       	printf("Second clone2() returned pid = %ld\n", (long) pid);

      	if (waitpid(pid, NULL, 0) == -1)    /* Wait for child */
     	      errExit("waitpid");


      	printf("Terminating now parent\n");
      	exit(EXIT_SUCCESS);
}
