#include "extra_functions.h"



#define DRV_NAME	"test function"
#define PFX		DRV_NAME ": "
#define DRV_VERSION	"0.0"

MODULE_AUTHOR("Chenxi Wang");
MODULE_DESCRIPTION("Provide test functions in extra module.");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION(DRV_VERSION);


int print_int_num(int num){
  printk(" Input number is : %d \n", num);

  return num;
}
EXPORT_SYMBOL(print_int_num);



// Kernel functions can only be static ?
static int init_this_module(void){
  printk(" Provide self defined kernel module function \n");
  
  print_int_num(77);

  return 0;
}


static void exit_this_module(void){
  printk("Delete extra_function module \n");
}

module_init(init_this_module);
module_exit(exit_this_module);
