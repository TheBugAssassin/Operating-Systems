#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/ktime.h>


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

static int lab1_show(struct seq_file *m, void *v) {
  /* some code here */
  struct task_struct *task = current;
  seq_printf(m, "Current Process PCB Information\n");
  seq_printf(m, "Name = %s\n", task->comm);
  seq_printf(m, "PID = %d\n", task->pid);
  seq_printf(m, "PPID = %d\n", task_ppid_nr(task));
 
  if (task->state == TASK_RUNNING)
    seq_printf(m, "State = Running\n");
  else if (task->state == TASK_INTERRUPTIBLE)
    seq_printf(m, "State = Waiting\n");
  else if (task->state == TASK_UNINTERRUPTIBLE)
    seq_printf(m, "State = Waiting\n");
  else if (task->state == TASK_STOPPED)
    seq_printf(m, "State = Not Running\n");

  seq_printf(m, "Real UID = %d\n", task->cred->uid.val);
  seq_printf(m, "Effective UID = %d\n", task->cred->euid.val);
  seq_printf(m, "Saved UID = %d\n", task->cred->suid.val);
  seq_printf(m, "Real GID = %d\n", task->cred->gid.val);
  seq_printf(m, "Effective GID = %d\n", task->cred->egid.val);
  seq_printf(m, "Saved GID = %d\n", task->cred->sgid.val);
  return 0;
}

static int lab1_open(struct inode *inode, struct file *file) {
  return single_open(file, lab1_show, NULL);
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops lab1_fops = {
  /* operation mapping */
  .proc_open = lab1_open,
  .proc_read = seq_read,
  .proc_lseek = seq_lseek,
  .proc_release = single_release,
};

#else
static const struct file_operations lab1_fops = {
  /* operation mapping */
  .owner = THIS_MODULE,
  .open = lab1_open,
  .read = seq_read,
  .llseek = seq_lseek,
  .release = single_release,
};
#endif

static int __init lab1_init(void) {
  /* create proc entry */
  if (!proc_create("lab1", 0, NULL, &lab1_fops)) {
    return -ENOMEM;
  }
  printk(KERN_INFO "lab1mod in\n");
  return 0;
}

static void __exit lab1_exit(void) {
  /* remove proc entry */
  remove_proc_entry("lab1", NULL);
  printk(KERN_INFO "lab1mod out\n");
}

MODULE_LICENSE("GPL");
module_init(lab1_init);
module_exit(lab1_exit);