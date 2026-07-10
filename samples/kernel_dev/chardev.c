// SPDX-License-Identifier: GPL-2.0-only
/*
 * chardev.c - Simple character device driver example.
 *
 * Demonstrates the key building blocks of a character device:
 * - alloc_chrdev_region() / unregister_chrdev_region() for dynamic major
 * - struct cdev and cdev_init() / cdev_add() / cdev_del()
 * - class_create() / device_create() to expose the node under /dev
 * - struct file_operations: open, release, read, write
 *
 * After loading:
 *   ls -l /dev/chardev_sample
 *   echo "test" > /dev/chardev_sample
 *   cat /dev/chardev_sample
 */

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kernel Dev");
MODULE_DESCRIPTION("Simple character device sample");

#define DEVICE_NAME	"chardev_sample"
#define CLASS_NAME	"chardev_sample"
#define BUF_SIZE	256

static dev_t devno;
static struct cdev sample_cdev;
static struct class *sample_class;
static struct device *sample_device;

/* Internal buffer shared between reads and writes */
static char kbuf[BUF_SIZE];
static size_t kbuf_len;
static DEFINE_MUTEX(kbuf_lock);

static int chardev_open(struct inode *inode, struct file *filp)
{
	pr_info("%s: open\n", DEVICE_NAME);
	return 0;
}

static int chardev_release(struct inode *inode, struct file *filp)
{
	pr_info("%s: release\n", DEVICE_NAME);
	return 0;
}

static ssize_t chardev_read(struct file *filp, char __user *ubuf,
			    size_t count, loff_t *ppos)
{
	size_t to_copy;
	ssize_t ret;

	if (mutex_lock_interruptible(&kbuf_lock))
		return -ERESTARTSYS;

	if (*ppos >= kbuf_len) {
		ret = 0;
		goto out;
	}

	to_copy = min(count, kbuf_len - (size_t)*ppos);
	if (copy_to_user(ubuf, kbuf + *ppos, to_copy)) {
		ret = -EFAULT;
		goto out;
	}

	*ppos += to_copy;
	ret = to_copy;
out:
	mutex_unlock(&kbuf_lock);
	return ret;
}

static ssize_t chardev_write(struct file *filp, const char __user *ubuf,
			     size_t count, loff_t *ppos)
{
	size_t to_copy = min(count, (size_t)(BUF_SIZE - 1));
	ssize_t ret;

	if (mutex_lock_interruptible(&kbuf_lock))
		return -ERESTARTSYS;

	if (copy_from_user(kbuf, ubuf, to_copy)) {
		ret = -EFAULT;
		goto out;
	}

	kbuf[to_copy] = '\0';
	kbuf_len = to_copy;
	*ppos = to_copy;
	pr_info("%s: received %zu bytes: %s\n", DEVICE_NAME, to_copy, kbuf);
	ret = to_copy;
out:
	mutex_unlock(&kbuf_lock);
	return ret;
}

static const struct file_operations chardev_fops = {
	.owner		= THIS_MODULE,
	.open		= chardev_open,
	.release	= chardev_release,
	.read		= chardev_read,
	.write		= chardev_write,
};

static int __init chardev_init(void)
{
	int ret;

	/* Allocate a dynamic major number */
	ret = alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME);
	if (ret < 0) {
		pr_err("%s: alloc_chrdev_region failed: %d\n", DEVICE_NAME, ret);
		return ret;
	}

	cdev_init(&sample_cdev, &chardev_fops);
	sample_cdev.owner = THIS_MODULE;

	ret = cdev_add(&sample_cdev, devno, 1);
	if (ret < 0) {
		pr_err("%s: cdev_add failed: %d\n", DEVICE_NAME, ret);
		goto err_unreg;
	}

	sample_class = class_create(CLASS_NAME);
	if (IS_ERR(sample_class)) {
		ret = PTR_ERR(sample_class);
		pr_err("%s: class_create failed: %d\n", DEVICE_NAME, ret);
		goto err_cdev;
	}

	sample_device = device_create(sample_class, NULL, devno, NULL,
				      DEVICE_NAME);
	if (IS_ERR(sample_device)) {
		ret = PTR_ERR(sample_device);
		pr_err("%s: device_create failed: %d\n", DEVICE_NAME, ret);
		goto err_class;
	}

	pr_info("%s: registered with major=%d minor=%d\n",
		DEVICE_NAME, MAJOR(devno), MINOR(devno));
	return 0;

err_class:
	class_destroy(sample_class);
err_cdev:
	cdev_del(&sample_cdev);
err_unreg:
	unregister_chrdev_region(devno, 1);
	return ret;
}

static void __exit chardev_exit(void)
{
	device_destroy(sample_class, devno);
	class_destroy(sample_class);
	cdev_del(&sample_cdev);
	unregister_chrdev_region(devno, 1);
	pr_info("%s: unregistered\n", DEVICE_NAME);
}

module_init(chardev_init);
module_exit(chardev_exit);
