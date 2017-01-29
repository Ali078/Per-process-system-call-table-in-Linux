#ifndef USER_IOCTL_H
#define USER_IOCTL_H

#include <linux/ioctl.h>

#define MAJOR_NUM (229)


#define IOCTL_SYSCALL_TBL_ADD_VECTOR _IOWR(MAJOR_NUM, 1, char *)
#define IOCTL_SYSCALL_TBL_REMOVE_VECTOR _IOWR(MAJOR_NUM, 2, char *)

#define IOCTL_SYSCALL_TBL_ADD_SYSCALL_TO_VECTOR _IOWR(MAJOR_NUM, 4, char *)
#define IOCTL_SYSCALL_TBL_REMOVE_SYSCALL_FROM_VECTOR _IOWR(MAJOR_NUM, 8, char *)


#define IOCTL_SYSCALL_TBL_ASSIGN_VECTOR_TO_PROCESS _IOWR(MAJOR_NUM, 16, char *)


#define IOCTL_SYSCALL_TBL_GET_VECTOR_ID_OF_PROCESS _IOWR(MAJOR_NUM, 32, char *)

#define IOCTL_SYSCALL_TBL_GET_INFO _IOWR(MAJOR_NUM, 64, char *)

#define IOCTL_SYSCALL_TBL_BLOCK _IOWR(MAJOR_NUM, 128, char *)
#define IOCTL_SYSCALL_TBL_UNBLOCK _IOWR(MAJOR_NUM, 256, char *)

#define MAX_SYSCALL_TBL_SIZE 10
#define MAX_SYSCALL_VECTOR_SIZE 10
#define MAX_NAME_LENGTH 64

#define NUM_SYSCALLS ((int) ((600) / 8)) + 1


struct syscall_vector_info {
        short vector_id;
        int size;
        char syscalls[20][MAX_NAME_LENGTH];
} typedef syscall_vector_info_t;

struct syscall_tbl_info {
        int size;
        syscall_vector_info_t  syscall_vector_info[10];
} typedef syscall_tbl_info_t;


#endif
