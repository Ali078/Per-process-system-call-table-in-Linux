#include <asm/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "user_ioctl.h"

#define SYSCALL_TBL_ADD_VECTOR 1

int list_all_syscall_vectors = 0;
int list_vector_id_of_given_ps = 0;
int manipulate_syscalls = 0;
int add_vector = 0;
int remove_vector = 0;
int add_syscall_to_vector = 0;
int remove_syscall_from_vector = 0;
int assign_vector_to_process = 0;
int block_syscall_in_vector = 0;
int unblock_syscall_in_vector = 0;

struct myargs {
	short vector_id;
	short syscall_num;
	char *name;
	pid_t pid;
	syscall_tbl_info_t *sys;
} typedef myargs;

inline void displayhelp(void)
{
	printf("Usage -- \n");
	printf("./user_ioctl [-c -p] [arguments]\n");
	printf("Default ./user_ioctl prints out the syscall_table\n");
	printf("./user_ioctl -p processid : gets vector id of process\n");
	printf("./user_ioctl -c value vectorid processid \n");
	printf("./user_ioctl -c 1 : add syscall vector\n");
	printf("./user_ioctl -c 2 vectorid : remove syscall vector\n");
	printf("./user_ioctl -c 4 vectorid syscallname: add syscall to vector\n");
	printf("./user_ioctl -c 8 vectorid syscallname : remove syscall from vector\n");
	printf("./user_ioctl -c 16 vectorid processid: assign vector to process\n");
	printf("./user_ioctl -c 32 vectorid syscallnum: block syscall in vector\n");
	printf("./user_ioctl -c 64 vectorid syscallnum: unblock syscall in vector\n");
}

int getcommandlineargs(struct myargs *args, int argc, char **argv)
{
	int c;
	int error_value = 0;
	int value = 0;
	while ((c = getopt(argc, argv, "cp")) != -1) {
		switch (c) {
		case 'c':
			manipulate_syscalls = 1;
			break;
		case 'p':
			list_vector_id_of_given_ps = 1;
			break;
		case 'h':
		case '?':
			displayhelp();
			return -1;
		default:
			list_all_syscall_vectors = 1;
			break;
		}
	}

	if ((list_vector_id_of_given_ps && manipulate_syscalls) ||
	     (list_vector_id_of_given_ps && list_all_syscall_vectors) ||
	     (manipulate_syscalls && list_all_syscall_vectors)) {

		printf("Combination of flags passed. Not allowed\n");
		displayhelp();
		return -1;

	}

	if (list_all_syscall_vectors) {
		if (argc > 1) {
			displayhelp();
			return -1;
		}
	}

	if (list_vector_id_of_given_ps) {
		if (argc < 3) {
			displayhelp();
			return -1;
		}
		args->pid = strtol(argv[optind++], NULL, 0);
	}

	if (manipulate_syscalls) {
		if (argc < 3) {
			displayhelp();
			return -1;
		}

		value = strtol(argv[optind++], NULL, 0);

		switch (value) {
		case 1:
			add_vector = 1;
			break;
		case 2:
			remove_vector = 1;
			if (argc < 4) {
				displayhelp();
				return -1;
			}
			args->vector_id = strtol(argv[optind++], NULL, 0);
			break;
		case 4:
			add_syscall_to_vector = 1;
			if (argc < 5) {
				displayhelp();
				return -1;
			}
			args->vector_id = strtol(argv[optind++], NULL, 0);
			args->name = argv[optind++];
			break;
		case 8:
			remove_syscall_from_vector = 1;
			if (argc < 5) {
				displayhelp();
				return -1;
			}
			args->vector_id = strtol(argv[optind++], NULL, 0);
			args->name = argv[optind++];
			break;
		case 16:
			assign_vector_to_process = 1;
			if (argc < 5) {
				displayhelp();
				return -1;
			}
			args->vector_id = strtol(argv[optind++], NULL, 0);
			args->pid = strtol(argv[optind++], NULL, 0);
			break;
		case 32:
			block_syscall_in_vector = 1;
			if (argc < 5) {
                                displayhelp();
                                return -1;
                        }
			args->vector_id = strtol(argv[optind++], NULL, 0);
                        args->syscall_num = strtol(argv[optind++], NULL, 0);
			break;
			
		case 64:
			unblock_syscall_in_vector = 1;
			if (argc < 5) {
                                displayhelp();
                                return -1;
                        }
			args->vector_id = strtol(argv[optind++], NULL, 0);
                        args->syscall_num = strtol(argv[optind++], NULL, 0);
			break;
		default:
			displayhelp();
			return -1;
			break;
		}


	}
	return error_value;
}




int main(int argc, char *argv[])
{
	int rc = 0;
	char ioctl_device_location[] = "/dev/hw3_ioctl_test_device";
	int fd;
	int i;
	int j;
	myargs send_arg;

	send_arg.vector_id = 0;
	send_arg.name = NULL;
	send_arg.pid = 0;

	if (argc == 1)
		list_all_syscall_vectors = 1;
	rc = getcommandlineargs(&send_arg, argc, argv);
	if (rc) {
		printf("The program terminated because there was some error "
		       "in reading command line arguments\n");
		return -1;
	}

/*
	printf("manipulate syscalls = %d\n", manipulate_syscalls);
	printf("vector id = %d\n", send_arg.vector_id);
	printf("pid = %d\n", send_arg.pid);
	printf("name = %s\n", send_arg.name);
	printf("add vector id bool = %d\n", add_vector);
	printf("remove vector = %d\n", remove_vector);
	printf("add syscall to vector =%d\n", add_syscall_to_vector);
	printf("remove syscall from vector = %d\n", remove_syscall_from_vector);
	printf("list_all_syscall_vectors= %d\n", list_all_syscall_vectors);
	printf("list_vector_id_of_given_ps= %d\n", list_vector_id_of_given_ps);
*/
	fd = open(ioctl_device_location, O_RDONLY);
	if (fd < 0) {
		printf("Is hw3_ioctl_device module loaded?\n");
		printf("mknod /dev/hw3_ioctl_test_device c 229 121\n");
		rc = -1;
		goto out;
	}

	if (add_vector) {
		rc = ioctl(fd, IOCTL_SYSCALL_TBL_ADD_VECTOR, &send_arg);
		if (rc < 0) {
			printf("Return value is less than 0. Some issues\n");
			goto out;
		}
	}
	if (remove_vector) {
		rc = ioctl(fd, IOCTL_SYSCALL_TBL_REMOVE_VECTOR, &send_arg);
		if (rc < 0) {
			printf("Return value is less than 0. Some issues\n");
			goto out;
		}
	}
	if (add_syscall_to_vector) {
		rc = ioctl(fd, IOCTL_SYSCALL_TBL_ADD_SYSCALL_TO_VECTOR, &send_arg);
		if (rc < 0) {
			printf("Return value is less than 0. Some issues\n");
			goto out;
		}

	}
	if (remove_syscall_from_vector) {
		rc = ioctl(fd, IOCTL_SYSCALL_TBL_REMOVE_SYSCALL_FROM_VECTOR,
			   &send_arg);
		if (rc < 0) {
			printf("Return value is less than 0. Some issues\n");
			goto out;
		}
	}

	if (assign_vector_to_process) {

		rc = ioctl(fd, IOCTL_SYSCALL_TBL_ASSIGN_VECTOR_TO_PROCESS,
			   &send_arg);
		if (rc < 0) {
			printf("Return value is less than 0. Some issues\n");
			goto out;
		}
	}


	if (list_all_syscall_vectors) {
		send_arg.sys = malloc(sizeof(syscall_tbl_info_t));
		rc = ioctl(fd, IOCTL_SYSCALL_TBL_GET_INFO, &send_arg);
		printf("\n**Printing the syscall table\n");
		for (i = 0 ; i < send_arg.sys->size ; ++i) {
			printf("%d -\t",
			       send_arg.sys->syscall_vector_info[i].vector_id);
			for (j = 0 ; j < send_arg.sys->syscall_vector_info[i].size ; ++j) {
				printf("%s\t", (char *)send_arg.sys->syscall_vector_info[i].syscalls[j]);
			}
			printf("\n");
		}
		printf("\nTable printed**\n\n");
		free(send_arg.sys);
	}
	if (list_vector_id_of_given_ps) {
		rc = ioctl(fd, IOCTL_SYSCALL_TBL_GET_VECTOR_ID_OF_PROCESS,
			   &send_arg);
		printf("Vector id of process %d = %d\n", send_arg.pid,
		       send_arg.vector_id);
	}
	
	if(block_syscall_in_vector) {
		rc = ioctl(fd, IOCTL_SYSCALL_TBL_BLOCK, &send_arg);
		if (rc < 0) {
                        printf("Return value is less than 0. Some issues\n");
                        goto out;
                }
                printf("Blocked syscall number %d in Vector id %d\n", send_arg.syscall_num,
                       send_arg.vector_id);
	}

	if(unblock_syscall_in_vector) {
                rc = ioctl(fd, IOCTL_SYSCALL_TBL_UNBLOCK, &send_arg);
                if (rc < 0) {
                        printf("Return value is less than 0. Some issues\n");
                        goto out;
                }
                printf("Unblocked syscall number %d in Vector id %d\n", send_arg.syscall_num,
                       send_arg.vector_id);
        }

out:
	if (fd)
		close(fd);
	exit(rc);
}

