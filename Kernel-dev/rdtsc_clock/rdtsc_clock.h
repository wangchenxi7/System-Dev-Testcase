
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ktime.h>
#include <linux/init.h>
//#include <linux/delay.h>
#include <asm-generic/delay.h>


void function_to_be_measured(int iteration);