#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/pid.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/dcache.h>
#include <asm/io.h>
#include <asm/fpu/types.h>
#include <asm/processor.h>
#include <time.h>

#define BUFFER_SIZE 1024

MODULE_LICENSE("GPL");

static struct mutex lock;
static struct dentry *debug_dir;
static struct dentry *debug_file;
static struct task_struct* task = NULL;
static struct inode* inode;
static struct fregs_state* fpu_context;
static int print_struct(struct seq_file *file, void *data);
static int print_error_0(struct seq_file *file, void *data);
static int print_error_1(struct seq_file *file, void *data);
static void print_inode(struct seq_file *file, struct task_struct *task);
static void print_fpu_context(struct seq_file *file, struct task_struct *task);

static int open_function(struct inode *inodep, struct file *filep){
   mutex_lock(&lock);
   printk(KERN_INFO "File has been opened");
   return 0;
}

static int release_function(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "File has been released successfully");
   mutex_unlock(&lock);
   return 0;
}

static ssize_t write_function(struct file *file, const char __user *buffer, size_t length, loff_t *ptr_offset) {
  char user_data[BUFFER_SIZE];
  unsigned long pid_number;
  copy_from_user(user_data, buffer, length);
  sscanf(user_data, "pid: %ld", &pid_number);
  struct pid* pid = find_get_pid(pid_number); 
  if(pid){ 
     task = get_pid_task(pid, PIDTYPE_PID);
        if(task->mm){
            single_open(file, print_struct, NULL);
        }else{
            single_open(file, print_error_0, NULL);
        }
    }else{
	single_open(file, print_error_1, NULL);
    }
  return strlen(user_data);
}

static struct file_operations fops = {
  .open = open_function,
  .read = seq_read,
  .write = write_function,
  .release = release_function
};

static int print_error_0(struct seq_file *file, void *data){
  seq_printf(file,"\nProcess exist but doesn't have associate memory\n");
  return 0;
}

static int print_error_1(struct seq_file *file, void *data){
  seq_printf(file,"\nProcess doesn't exist\n");
  return 0;
}

static int __init mod_init(void) {
  mutex_init(&lock);
  debug_dir = debugfs_create_dir("lab2", NULL);
  debug_file = debugfs_create_file("lab2file", 0777, debug_dir, NULL, &fops);
  return 0;
}

static void __exit mod_exit(void) {
  debugfs_remove_recursive(debug_dir);
}


static int print_struct(struct seq_file *file, void *data) {
  print_inode(file, task);
  print_fpu_context(file, task);
  return 0;
}
static void print_time(struct seq_file *file,unsigned long time, int mode){
  unsigned long t = time;
  unsigned int year = 1970;
  unsigned int mounth;
  unsigned int day;
  unsigned int hour;
  unsigned int minutes;
  unsigned int seconds;
  unsigned int vgo;
  unsigned int numberday;
  while(t>=31536000){
    if (year%4!=0||(year%100==0 && year%400!=0)){
      t=t-31536000;
      year++;
    } else {
      t=t-31622400;
      year++;
    }
  }
  if (year%4!=0||(year%100==0 && year%400!=0)){
    vgo=0;
  } else {
    vgo=1;
  }
  if (t%86400==0){
  numberday=t/86400;
  t=t-numberday*86400;
  } else {
  numberday=t/86400+1;
  t=t-(numberday-1)*86400;
  } 
  if (numberday<=31){ 
    day=numberday;
    mounth=1;
  }
  if (numberday>31 && numberday<=59+vgo){
    day=numberday-31-vgo;
    mounth=2;
  }
  if (numberday>59+vgo && numberday<=90+vgo){
    day=numberday-59-vgo;
    mounth=3;
  }
  if (numberday>90+vgo && numberday<=120+vgo){
    day=numberday-90-vgo;
    mounth=4;
  }
  if (numberday>120+vgo && numberday<=151+vgo){
    day=numberday-120-vgo;
    mounth=5;
  }
  if (numberday>151+vgo && numberday<=181+vgo){
    day=numberday-151-vgo;
    mounth=6;
  }
  if (numberday>181+vgo && numberday<=212+vgo){
    day=numberday-181-vgo;
    mounth=7;
  }
  if (numberday>212+vgo && numberday<=243+vgo){
    day=numberday-212-vgo;
    mounth=8;
  }
  if (numberday>243+vgo && numberday<=273+vgo){
    day=numberday-243-vgo;
    mounth=9;
  }
  if (numberday>273+vgo && numberday<=304+vgo){
    day=numberday-273-vgo;
    mounth=10;
  }
  if (numberday>304+vgo && numberday<=334+vgo){
    day=numberday-304-vgo;
    mounth=11;
  }
  if (numberday>334+vgo && numberday<=365+vgo){
    day=numberday-334-vgo;
    mounth=12;
  }
  hour=t/3600;
  t=t-hour*3600;
  minutes = t/60;
  t=t-minutes*60;
  seconds=t; 
  if (mode==1){
    seq_printf(file,"\tlast opening time = %u:%u:%u  %u:%u:%u\n",year,mounth,day,hour,minutes,seconds);
  } else {
    seq_printf(file,"\trecent changes time = %u:%u:%u  %u:%u:%u\n",year,mounth,day,hour,minutes,seconds);
  }
}

static void print_fpu_context(struct seq_file *file, struct task_struct *task) {
  fpu_context = &(task->thread.fpu.state.fsave);
  seq_printf(file,"\nFPU context {\n");	
  seq_printf(file,"\tFPU Control Word =%u\n",fpu_context->cwd);
  seq_printf(file,"\tFPU Status Word =%u\n",fpu_context->swd);
  seq_printf(file,"\tFPU Tag Word =%u\n",fpu_context->twd);
  seq_printf(file,"\tFPU IP Offset =%u\n",fpu_context->fip);
  seq_printf(file,"\tFPU IP Selector =%u\n",fpu_context->fcs);
  seq_printf(file,"\tFPU Operand Pointer Offset =%u\n",fpu_context->foo);
  seq_printf(file,"\tFPU Operand Pointer Selector =%u\n",fpu_context->fos);
  seq_printf(file,"}\n");	
}


static void print_inode(struct seq_file *file, struct task_struct *task) {
  inode = task->mm->mmap->vm_file->f_inode;
  seq_printf(file,"\ninode {\n");	
  seq_printf(file,"\tinode number = %lu\n",inode->i_ino);
  seq_printf(file,"\tcounter links = %u\n",inode->i_count);
  seq_printf(file,"\tcount hard link = %u\n",inode->i_nlink);
  seq_printf(file,"\tsize file(byte) = %llu\n",inode->i_size);
  print_time(file,inode->i_atime.tv_sec,1);
  print_time(file,inode->i_ctime.tv_sec,0);
  seq_printf(file,"}\n");	
}


module_init(mod_init);
module_exit(mod_exit);
