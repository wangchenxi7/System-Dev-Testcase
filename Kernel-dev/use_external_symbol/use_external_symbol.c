#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>


MODULE_AUTHOR("Chenxi Wang");
MODULE_DESCRIPTION("Use external symbols from otehr module.");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("0.0");


// Declare the External symbols
// If can't include the header who declares this function,
// We have to declare it as exteren
extern int print_int_num(int num);  

static int init_this_module(void){
  printk(" Invoke external functions from other modules \n");
  
  print_int_num(77);

  return 0;
}


static void exit_this_module(void){
  printk("Delete use_external_symbol module \n");
}

module_init(init_this_module);
module_exit(exit_this_module);

