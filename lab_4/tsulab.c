#include<linux/kernel.h>
#include <linux/module.h> /* Needed by all modules */
#include <linux/printk.h> /* Needed for pr_info() */
#include<linux/proc_fs.h>
#include<linux/uaccess.h>
#include <linux/version.h>
#define procfs_name "tsu"
static struct proc_dir_entry *our_proc_file = NULL;


bool ANSWER_FLAG, STOP_FLAG;

MODULE_LICENSE("GPL");

static ssize_t procfile_read(struct file *file_pointer, char __user *buffer, size_t buffer_length, loff_t* offset) {
    
    if(STOP_FLAG){
	STOP_FLAG = !STOP_FLAG;
	return 0;
    }
    else{
	char s[5];
    	if(ANSWER_FLAG){
            strcpy(s, "Yes\n");
    	}
    	else{
            strcpy(s, "No\n");
    	}
   	ANSWER_FLAG = !ANSWER_FLAG;
    	size_t len = strlen(s);
    	copy_to_user(buffer, s, len);
    	pr_info("procfile read %s\n", file_pointer->f_path.dentry->d_name.name);
	STOP_FLAG = !STOP_FLAG;
    	return len;
    }
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops proc_file_fops = {
    .proc_read = procfile_read,
};
#else
static const struct file_operations proc_file_fops = {
    .read = procfile_read,
};
#endif

static int __init procfs1_init(void) {
    our_proc_file = proc_create(procfs_name, 0644, NULL, &proc_file_fops);
    ANSWER_FLAG = true;
    STOP_FLAG = false;
    pr_info("Welcome to Tomsk\n");
    return 0;
}

static void __exit procfs1_exit(void){
    proc_remove(our_proc_file);
    pr_info("Unloding the TSU Linux Module\n");
}

module_init(procfs1_init);
module_exit(procfs1_exit);
