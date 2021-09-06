/**
 * @file kernel_version_utsname.c
 * 
 * @author your name (you@domain.com)
 * @brief  utsname is used in kernel space. usualy used by kernel module. 
 * 	It gets information from /proc/version, which is already correct.
 * 	This is much more accurate than LINUX_VERSION_CODE
 * @version 0.1
 * @date 2021-09-06
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/utsname.h>


static int __init kernel_version_utsname_init(void)
{
        printk(KERN_INFO "Kernel version %s\n", utsname()->version);
        printk(KERN_INFO "Kernel release %s\n", utsname()->release);

        return 0;
}

static void __exit kernel_version_utsname_eixt(void)
{
        printk(KERN_INFO "%s, exit module \n", __func__);

        return;
}

module_init(kernel_version_utsname_init);
module_exit(kernel_version_utsname_eixt);

MODULE_LICENSE("GPL");