#include <linux/atomic.h> 
#include <linux/cdev.h> 
#include <linux/delay.h> 
#include <linux/device.h> 
#include <linux/fs.h> 
#include <linux/init.h> 
#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/printk.h> 
#include <linux/types.h> 
#include <linux/uaccess.h> 
#include <linux/version.h> 
#include <asm/errno.h> 
#include "kfetch.h"
#include <linux/sysinfo.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/sched.h>
#include <linux/utsname.h>
#include <asm/page.h>
#include <linux/ktime.h>

static int kfetch_open(struct inode *, struct file *); 
static int kfetch_release(struct inode *, struct file *); 
static ssize_t kfetch_read(struct file *, char __user *, size_t, loff_t *); 
static ssize_t kfetch_write(struct file *, const char __user *, size_t, loff_t *); 

#define SUCCESS 0 
#define DEVICE_NAME "kfetch"
#define BUF_LEN 80
 
static int major;
 
enum { 
    CDEV_NOT_USED = 0, 
    CDEV_EXCLUSIVE_OPEN = 1, 
}; 
 
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED); 
 
static char msg[BUF_LEN + 1];

static struct class *cls; 
 
static struct file_operations kfetch_ops = { 
    .owner = THIS_MODULE,
    .read = kfetch_read, 
    .write = kfetch_write, 
    .open = kfetch_open, 
    .release = kfetch_release, 
}; 

static int __init kfetch_init(void) 
{ 
    major = register_chrdev(0, DEVICE_NAME, &kfetch_ops); 
 
    if (major < 0) { 
        pr_alert("Registering char device failed with %d\n", major); 
        return major; 
    } 
 
    pr_info("I was assigned major number %d.\n", major); 
    cls = class_create(THIS_MODULE, DEVICE_NAME); 
    device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME); 
    pr_info("Device created on /dev/%s\n", DEVICE_NAME); 

    return SUCCESS; 
} 

static void __exit kfetch_exit(void) 
{ 
    device_destroy(cls, MKDEV(major, 0)); 
    class_destroy(cls); 
 
    /* Unregister the device */ 
    unregister_chrdev(major, DEVICE_NAME); 
} 

static int kfetch_open(struct inode *inode, struct file *file) 
{ 
    static int counter = 0; 
 
    if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN)) 
        return -EBUSY; 
 
    sprintf(msg, "I already told you %d times Hello world!\n", counter++); 
    try_module_get(THIS_MODULE);
     
    return SUCCESS; 
} 


static int kfetch_release(struct inode *inode, struct file *file) 
{  
    atomic_set(&already_open, CDEV_NOT_USED); 
    module_put(THIS_MODULE); 

    return SUCCESS; 
} 

int mask[KFETCH_NUM_INFO] = {1, 1, 1, 1, 1, 1};

static ssize_t kfetch_read(struct file *filp, 
                           char __user *buffer, 
                           size_t length, 
                           loff_t *offset) 

{ 
    char kf_buffer[1000];
    memset(kf_buffer, 0, sizeof(kf_buffer) - 1);
    // line 1
    
    strncat(kf_buffer, "                   \0", sizeof(kf_buffer) - strlen(kf_buffer) - 1);
    strncat(kf_buffer, utsname()->nodename, sizeof(kf_buffer) - strlen(kf_buffer) - 1); /*host name*/
    strncat(kf_buffer, "\n\0", strlen("\n\0"));

    // line 2
    strncat(kf_buffer, "        .-.        \0", sizeof(kf_buffer) - strlen(kf_buffer) - 1);
    char SplitLine[strlen(utsname()->nodename)+1];
    memset(SplitLine, '-', sizeof(SplitLine) - 1);
    SplitLine[sizeof(SplitLine) - 1] = '\0';
    strncat(kf_buffer, SplitLine, strlen(SplitLine)); 
    strncat(kf_buffer, "\n\0", strlen("\n\0"));

    int info = -1;
    //line 3-8
    for(int i = 3; i < 9; i++){
        switch(i){
            case 3:
                strncat(kf_buffer, "       (.. |       \0", sizeof(kf_buffer) - strlen(kf_buffer) - 1);
                break;
            case 4:
                strncat(kf_buffer, "       <>  |       \0", sizeof(kf_buffer) - strlen(kf_buffer) - 1);
                break;
            case 5:
                strncat(kf_buffer, "      / --- \\      \0", sizeof(kf_buffer) - strlen(kf_buffer) - 1);
                break;
            case 6:
                strncat(kf_buffer, "     ( |   | |     \0", sizeof(kf_buffer) - strlen(kf_buffer) - 1);
                break;
            case 7:
                strncat(kf_buffer, "   |\\\\_)___/\\)/\\   \0", sizeof(kf_buffer) - strlen(kf_buffer) - 1);
                break;
            case 8:
                strncat(kf_buffer, "  <__)------(__/   \0", sizeof(kf_buffer) - strlen(kf_buffer) - 1);
                break;
        }
        //printk("%d", info);
        while(info<5){
            info++;
            if (mask[info] == 1){
	        if (info == 0){// kernel release
	            strncat(kf_buffer, "Kernel:   \0", sizeof(kf_buffer) - strlen(kf_buffer) - 1);
		    strncat(kf_buffer, utsname()->release, sizeof(kf_buffer) - strlen(kf_buffer) - 1);
			
		    printk("Kernel: %s\n", utsname()->release);
		}
		else if (info == 1){ // CPU model name
	            unsigned int cpu = 0;
		    struct cpuinfo_x86 *c;
		    for_each_online_cpu(cpu){
		        c = &cpu_data(cpu);
			strncat(kf_buffer, "CPU:      \0",  sizeof(kf_buffer) - strlen(kf_buffer) - 1);
		        strncat(kf_buffer, c->x86_model_id, sizeof(kf_buffer) - strlen(kf_buffer) - 1);
			    
		        printk("CPU: %s\n", c->x86_model_id);
		        break;
		    }
		}
		else if (info == 2){ // number of CPU cores
		    char online_cpu[10];
		    char total_cpu[10];
		    snprintf(online_cpu, sizeof(online_cpu), "%d", num_online_cpus());
		    snprintf(total_cpu, sizeof(online_cpu), "%d", nr_cpu_ids);
	 	    strncat(kf_buffer, "CPUs:     \0", sizeof(kf_buffer) - strlen(kf_buffer) - 1);
		    strncat(kf_buffer, online_cpu, sizeof(kf_buffer) - strlen(kf_buffer) - 1);
		    strncat(kf_buffer, " / ", sizeof(kf_buffer) - strlen(kf_buffer) - 1);
		    strncat(kf_buffer, total_cpu, sizeof(kf_buffer) - strlen(kf_buffer) - 1);
			
		    printk("CPUs: %d / %d", num_online_cpus(), nr_cpu_ids);
		}
		else if (info == 3){ // Mem
		    struct sysinfo sys_info;
		    si_meminfo(&sys_info);
		    unsigned long int total_mem = sys_info.totalram << (PAGE_SHIFT - 10);	
		    total_mem = total_mem / 1024;
		    unsigned long int free_mem = sys_info.freeram << (PAGE_SHIFT - 10);
		    free_mem = free_mem / 1024;
		    char free_mem_arr[10];
		    char total_mem_arr[10];
		    snprintf(free_mem_arr, sizeof(free_mem_arr), "%ld", free_mem);
		    snprintf(total_mem_arr, sizeof(total_mem_arr), "%ld", total_mem);
		    strncat(kf_buffer, "Mem:      \0", sizeof(kf_buffer) - strlen(kf_buffer) - 1);
		    strncat(kf_buffer, free_mem_arr, sizeof(kf_buffer) - strlen(kf_buffer) - 1);
		    strncat(kf_buffer, " MB / \0", sizeof(kf_buffer) - strlen(kf_buffer) - 1);
		    strncat(kf_buffer, total_mem_arr, sizeof(kf_buffer) - strlen(kf_buffer) - 1);
		    strncat(kf_buffer, " MB\0", sizeof(kf_buffer) - strlen(kf_buffer) - 1);
			
		    printk("Mem: %lu MB / %lu MB", free_mem, total_mem);

		}
		else if (info == 4){ // process
		    struct task_struct* task;
		    size_t count = 0;
		    for_each_process(task) {
			++count;
		    }
		    char process[10];
		    snprintf(process, sizeof(process), "%ld", count);
		    strncat(kf_buffer, "Procs:    \0", sizeof(kf_buffer) - strlen(kf_buffer) - 1);
		    strncat(kf_buffer, process, sizeof(kf_buffer) - strlen(kf_buffer) - 1);

		    printk("process: %ld\n", count);
		}
		else if (info == 5){ // uptime
		    s64 uptime;
		    uptime = ktime_divns(ktime_get_coarse_boottime(), NSEC_PER_SEC);
		    char uptime_string[10];
		    snprintf(uptime_string, sizeof(uptime_string), "%lld", uptime/60);
		    strncat(kf_buffer, "Uptime:   \0", sizeof(kf_buffer) - strlen(kf_buffer) - 1);
		    strncat(kf_buffer, uptime_string, sizeof(kf_buffer) - strlen(kf_buffer) - 1);
		    strncat(kf_buffer, " mins\0", sizeof(kf_buffer) - strlen(kf_buffer) - 1);
			
		    printk("uptime: %lld mins\n", uptime/60);

		}
                break;
            }
        }
        if(i != 8){
        	strncat(kf_buffer, "\n\0", strlen("\n\0"));
        }
        
    }
    printk("%s\n", kf_buffer);
    ssize_t ret = copy_to_user(buffer, kf_buffer, strlen(kf_buffer));
    
    if (ret>0){
        printk("partial data copy failed\n");
        pr_alert("Failed to copy data to user");
        return -EFAULT;
    }
    
    return strlen(kf_buffer);
} 

static ssize_t kfetch_write(struct file *filp, 
                           const char __user *buffer, 
                           size_t length, 
                           loff_t *offset) 
{ 
    int mask_info;

    if (copy_from_user(&mask_info, buffer, length)){
        pr_alert("Failed to copy data from user");
        return 0;
    }
    
    /* setting the information mask */
    if (mask_info & KFETCH_RELEASE){
        mask[0] = 1;
        printk("%s", "KFETCH_RELEASE\n");
    }
    else{
        mask[0] = 0;
    }

    if (mask_info & KFETCH_NUM_CPUS){
        mask[2] = 1;
        printk("%s", "KFETCH_NUM_CPUS\n");
    }
    else{
        mask[2] = 0;
    }

    if (mask_info & KFETCH_CPU_MODEL){
        mask[1] = 1;
        printk("%s", "KFETCH_CPU_MODEL\n");
    }
    else{
        mask[1] = 0;
    }

    if (mask_info & KFETCH_MEM){
        mask[3] = 1;
        printk("%s", "KFETCH_MEM\n");
    }
    else{
        mask[3] = 0;
    }

    if (mask_info & KFETCH_UPTIME){
        mask[5] = 1;
        printk("%s", "KFETCH_UPTIME\n");
    }
    else{
        mask[5] = 0;
    }

    if (mask_info & KFETCH_NUM_PROCS){
        mask[4] = 1;
        printk("%s", "KFETCH_NUM_PROCS\n");
    }
    else{
        mask[4] = 0;
    }
    return 0;
} 

module_init(kfetch_init); 
module_exit(kfetch_exit);
 
MODULE_LICENSE("GPL");
