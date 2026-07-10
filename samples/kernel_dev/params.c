// SPDX-License-Identifier: GPL-2.0-only
/*
 * params.c - Kernel module parameters example.
 *
 * Demonstrates how to pass parameters to a kernel module at load time and
 * how to expose them as read/write sysfs attributes under
 * /sys/module/params/parameters/.
 *
 * Supported parameter types: int, charp (string), bool, array.
 *
 * Load with custom values:
 *   insmod params.ko greeting="Hi" count=3 verbose=1
 * Read back at runtime:
 *   cat /sys/module/params/parameters/greeting
 *   cat /sys/module/params/parameters/count
 * Modify at runtime (0644 params only):
 *   echo "Hello" > /sys/module/params/parameters/greeting
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kernel Dev");
MODULE_DESCRIPTION("Kernel module parameters sample");

/* String parameter (0644 = readable and writable via sysfs) */
static char *greeting = "Hello, Kernel!";
module_param(greeting, charp, 0644);
MODULE_PARM_DESC(greeting, "A greeting string (default: \"Hello, Kernel!\")");

/* Integer parameter */
static int count = 1;
module_param(count, int, 0644);
MODULE_PARM_DESC(count, "Number of times to print the greeting (default: 1)");

/* Boolean parameter */
static bool verbose;
module_param(verbose, bool, 0444);
MODULE_PARM_DESC(verbose, "Enable verbose output (default: false)");

static int __init params_init(void)
{
	int i;

	if (verbose)
		pr_info("params: initialising with greeting=\"%s\", count=%d\n",
			greeting, count);

	for (i = 0; i < count; i++)
		pr_info("params: %s\n", greeting);

	return 0;
}

static void __exit params_exit(void)
{
	pr_info("params: module unloaded\n");
}

module_init(params_init);
module_exit(params_exit);
