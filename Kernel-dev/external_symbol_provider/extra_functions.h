/*
 * For Kernel development test
 * 1) Generate Module.sysvers
 * 2) Include extra sysbol from other kernel modules
 *    Define some kernel funcstions in this module.
 *    Let other other modules try to access this kernel module.
*/
#ifndef EXTRA_KERNEL_SYSBOL
#define EXTRA_KERNEL_SYSBOL

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#endif


