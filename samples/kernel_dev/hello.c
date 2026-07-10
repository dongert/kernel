// SPDX-License-Identifier: GPL-2.0-only
/*
 * hello.c - A minimal kernel module example.
 *
 * This module demonstrates the basic structure of a Linux kernel module:
 * - module_init() / module_exit() lifecycle hooks
 * - pr_info() for kernel log output
 * - MODULE_LICENSE / MODULE_AUTHOR / MODULE_DESCRIPTION macros
 *
 * Load:   insmod hello.ko
 * Unload: rmmod hello
 * Log:    dmesg | tail
 */

#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kernel Dev");
MODULE_DESCRIPTION("Hello World kernel module sample");

static int __init hello_init(void)
{
	pr_info("hello: module loaded\n");
	return 0;
}

static void __exit hello_exit(void)
{
	pr_info("hello: module unloaded\n");
}

module_init(hello_init);
module_exit(hello_exit);
