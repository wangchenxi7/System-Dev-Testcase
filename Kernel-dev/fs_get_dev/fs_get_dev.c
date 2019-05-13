/**
* Kernel module testcase
* Purpose : get a Block Device by its path
* get the "struct block_device" information of /dev/sdb1 
*/

#include <linux/kernel.h>
#include <linux/module.h>

//for stackbd
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <trace/events/block.h>



#define STACKBD_BDEV_MODE (FMODE_READ | FMODE_WRITE | FMODE_EXCL)


// Kernel functions can only be static ?
static int init(){
  printk("Init self build kernel \n");

  return 0;
}


static int exit(){
  printk("Exit self build kernel \n");

  return 0;
}

module_init(init());
module_exit(exit());

