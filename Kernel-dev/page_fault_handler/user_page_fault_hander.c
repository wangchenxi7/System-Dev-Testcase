/**
 * Test the user space page fault handler function.
 * 
 * Page fault can be:
 *  1) The first time to touch the virtual memory. Need to map physical memory to the virtual memory range. 
 *  2) The mapped physical page is swapped out.
 * 
 * 
 */


#define _GNU_SOURCE  1  // Enable the macro defination of mremap prot flags

#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>  
#include <sys/errno.h>

#include "stdint.h"
#include "stdio.h"
#include "stddef.h"


typedef enum {true, false} bool;

extern errno;





