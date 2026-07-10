// SPDX-License-Identifier: GPL-2.0-only
/*
 * procfs.c - /proc filesystem interface example.
 *
 * Demonstrates how to create and manage a /proc entry using the seq_file
 * interface, which is the preferred approach for multi-line or large output.
 *
 * A read-only entry /proc/sample_info is created that reports some basic
 * kernel and module information.
 *
 * After loading:
 *   cat /proc/sample_info
 */

#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/utsname.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kernel Dev");
MODULE_DESCRIPTION("procfs seq_file interface sample");

#define PROC_ENTRY_NAME "sample_info"

static int sample_show(struct seq_file *m, void *v)
{
	seq_printf(m, "Module:    %s\n", THIS_MODULE->name);
	seq_printf(m, "Kernel:    %s %s\n",
		   utsname()->sysname, utsname()->release);
	seq_printf(m, "Jiffies:   %lu\n", jiffies);
	seq_printf(m, "HZ:        %d\n", HZ);
	return 0;
}

/*
 * single_open() is a convenience wrapper from <linux/seq_file.h> that
 * handles the iterator boilerplate for single-page /proc entries.
 */
static int sample_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, sample_show, NULL);
}

static const struct proc_ops sample_proc_ops = {
	.proc_open	= sample_open,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
};

static struct proc_dir_entry *proc_entry;

static int __init procfs_init(void)
{
	proc_entry = proc_create(PROC_ENTRY_NAME, 0444, NULL,
				 &sample_proc_ops);
	if (!proc_entry) {
		pr_err("procfs: failed to create /proc/%s\n", PROC_ENTRY_NAME);
		return -ENOMEM;
	}

	pr_info("procfs: /proc/%s created\n", PROC_ENTRY_NAME);
	return 0;
}

static void __exit procfs_exit(void)
{
	proc_remove(proc_entry);
	pr_info("procfs: /proc/%s removed\n", PROC_ENTRY_NAME);
}

module_init(procfs_init);
module_exit(procfs_exit);
