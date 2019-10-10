/**
 * hrtimer is the kernel level most high-resolution timer.
 * It's defined in include/linux/hrtimer.h
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/init.h>


// Declare a global timer
// Single instane across all the files.
extern  struct hrtimer hr_timer;