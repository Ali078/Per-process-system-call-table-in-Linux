#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/fs.h>
#include "syscall_tbl.h" 
#include <linux/string.h>


#define WRAPPED -8000
#define ENOT_SUPPORTED -9500

#define READ_SYSNUM 0
#define OPEN_SYSNUM 2
#define CLOSE_SYSNUM 3
#define MKDIR_SYSNUM 83
#define LINK_SYSNUM 86
#define UNLINK_SYSNUM 87
#define CHMOD_SYSNUM 90
#define RENAME_SYSNUM 82
#define RMDIR_SYSNUM 84

extern int register_syscall(char * name, unsigned long fptr, short syscall_num, struct module * module);
extern int unregister_syscall(char * name);

struct file *filp = NULL;
char *default_file = "default_file.txt";

asmlinkage long read2(unsigned int fd, char __user *buf, size_t count)       // override read system call
{	
        int ret = 1,err_s=0;
	char * kbuf = NULL;
	mm_segment_t oldfs;   

	printk("Inside read 2 system call\n");
  
        kbuf = kmalloc(100, GFP_KERNEL);
	if(!kbuf) {
		printk("Memory allocation failed\n");
		ret = WRAPPED;
		goto out;
	}
	oldfs = get_fs();
	set_fs(KERNEL_DS);
	err_s = vfs_read(filp, kbuf, 30, &filp->f_pos);
	if(err_s < 0) {
		printk("failed to read\n");
		ret = WRAPPED;
                goto out;
	}
        ret = err_s;      //bytes returned 
	set_fs(oldfs);
        kbuf[30] = '\0';
        err_s = copy_to_user(buf,kbuf,30);
        if(err_s<0)
         {
             printk("Failed copy to user \n");
             ret = WRAPPED;
             goto out;
         }

	printk("Content read: %s\n", kbuf);

	kfree(kbuf);
        
out:     
	return ret;
}

/*
asmlinkage long write2(unsigned int fd, char __user *buf, size_t count)      // override write system call
{
	int ret = 1,err_s=0;
        mm_segment_t oldfs;

        printk("Inside write2 system call\n");
        
        oldfs = get_fs();
        set_fs(KERNEL_DS);
        
        err_s = vfs_write(filp, "Adding content to default file",30, &filp->f_pos);
        if(err_s < 0) {
                printk("failed to write\n");
                ret = WRAPPED;
                goto out;
        }
        ret = err_s;      //bytes written 
        set_fs(oldfs);
        

        //printk("Content written: %s\n", kbuf);


out:
        return ret;
 
}	
*/

asmlinkage long open2(const char __user *filename, int flags, umode_t mode )    // override open system call
{

	int ret = 1;
	printk("Inside open2 system call\n");
        filp = filp_open(default_file,flags,mode);
        
        if (!(filp) || IS_ERR(filp))
        {
                printk("KERNEL ERROR: Cannot open file (%d)",(int)PTR_ERR(filp));
             	return WRAPPED;
        }
       
        return ret;          //return dummy fd value

	
}

asmlinkage long close2( unsigned int fd)        // override close system call
{
	int ret = 1, err_s = 0;
	printk("Inside close2 system call\n");
        
        err_s = filp_close(filp,NULL);       
        if(err_s < 0) {
                printk("failed to close\n");
                ret = WRAPPED;
                goto out;
        }

out:
        return ret;
}


asmlinkage long mkdir2(const char __user *pathname, umode_t mode)
{
        printk("Inside mkdir2 system call\n");
        
        return WRAPPED;
}

asmlinkage long link2(const char __user *oldname,
                                 const char __user *newname)
{
        printk("Inside link2 system call\n");
        return WRAPPED;
}

asmlinkage long unlink2(const char __user *pathname)
{
        printk("Inside unlink2 system call\n");
        return WRAPPED;
}

asmlinkage long chmod2(const char __user *filename, umode_t mode)
{
        printk("Inside chmod2 system call\n");
        return WRAPPED;
}

asmlinkage long rename2(const char __user *oldname,                      //modified system call          
                                 const char __user *newname)
{
        int err_s = 0;
        char *kbuf = NULL;
        char *ptr = NULL;
        printk("Inside rename2 system call\n");

        kbuf = kmalloc(100,GFP_KERNEL);
        if (!kbuf)
        {
          printk("Memory allocation failed\n");
          return WRAPPED;
        }
         
        err_s = copy_from_user(kbuf,oldname,strlen(oldname));
        if (err_s < 0)
        {
           printk("copy from user failed\n");
           kfree(kbuf);
           return WRAPPED;
        }
        kbuf[strlen(oldname)] = '\0';
     
        printk("filename to rename: %s\n",kbuf);

         
        ptr = strsep(&kbuf,".");
        if(kbuf==NULL)
        {
           return WRAPPED;
        }
        
        if(strcmp(kbuf,"protect")==0)
        {
            printk("PROTECTED FILE . CANNOT BE RENAMED\n");
            return ENOT_SUPPORTED;
        }
        else 
       	{
            return WRAPPED;
        }
      
}

asmlinkage long rmdir2(const char __user *pathname)                         //modified system call
                                 
{
        int err_s = 0;
        char *kbuf = NULL;
        char *ptr = NULL;
	
        printk("Inside rmdir2 system call\n");
        kbuf = kmalloc(100,GFP_KERNEL);
        if (!kbuf)
        {
          printk("Memory allocation failed\n");
          return WRAPPED;
        }

        err_s = copy_from_user(kbuf,pathname,strlen(pathname));
        if (err_s < 0)
        {
           printk("copy from user failed\n");
           kfree(kbuf);
           return WRAPPED;
        }
        kbuf[strlen(pathname)] = '\0';

        printk("directory to remove: %s\n",kbuf);


        ptr = strsep(&kbuf,".");
        if(kbuf==NULL)
        {
           return WRAPPED;
        }

        if(strcmp(kbuf,"protect")==0)
        {
            printk("PROTECTED DIRECTORY . CANNOT BE REMOVED\n");
            return ENOT_SUPPORTED;
        }
        else
        {
            return WRAPPED;
        }

}



/*
asmlinkage long clear_file(unsigned int fd, char __user *buf, size_t count)
{
       int a = 5,b=10; 
       printk("Inside new clear file system call\n");
       return a+b; 
}
*/

static int __init init_test_syscalls(void)
{
       register_syscall("read2", (unsigned long) read2, READ_SYSNUM, THIS_MODULE);
       register_syscall("open2", (unsigned long)open2, OPEN_SYSNUM, THIS_MODULE);
       register_syscall("close2", (unsigned long)close2, CLOSE_SYSNUM, THIS_MODULE);
       
       register_syscall("mkdir2", (unsigned long)mkdir2, MKDIR_SYSNUM, THIS_MODULE);
       register_syscall("link2", (unsigned long)link2, LINK_SYSNUM, THIS_MODULE);
       register_syscall("unlink2", (unsigned long)unlink2, UNLINK_SYSNUM, THIS_MODULE);
       register_syscall("chmod2", (unsigned long)chmod2, CHMOD_SYSNUM, THIS_MODULE);
       
       register_syscall("rename2", (unsigned long)rename2, RENAME_SYSNUM, THIS_MODULE);
       register_syscall("rmdir2", (unsigned long)rmdir2, RMDIR_SYSNUM, THIS_MODULE);     
   
       printk("REGISTERED SYSTEM CALLS\n");
       return 0;
}


static void __exit exit_test_syscalls(void)
{
     unregister_syscall("read2");
     unregister_syscall("open2");
     unregister_syscall("close2");
   
     unregister_syscall("mkdir2");
     unregister_syscall("link2");
     unregister_syscall("unlink2");
     unregister_syscall("chmod2");

     unregister_syscall("rename2");
     unregister_syscall("rmdir2");
}


MODULE_AUTHOR("Group 1*");
MODULE_DESCRIPTION("test system calls module");
MODULE_LICENSE("GPL");

module_init(init_test_syscalls);
module_exit(exit_test_syscalls);
