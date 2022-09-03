/**
 * @file register_user_kernel_shared_buf.c
 * @author Chenxi (you@domain.com)
 * @brief Let user(app) register a range of virtual memory, 
 * and then share it with the kernel.
 * 
 * @version 0.1
 * @date 2022-08-31
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>  
#include <sys/errno.h>

#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"
#include <string.h>


#define SYS_USER_KERNEL_BUF	457

#ifndef M
  #define M 1024*1024UL
#endif

typedef enum {true, false} bool;

/**
 * Reserve memory at fixed address 
 */
static char* reserve_anon_memory(char* requested_addr, unsigned long bytes, bool fixed) {
	char * addr;
	int flags;

	flags = MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS;   
	if (fixed == true) {
			printf("Request fixed addr 0x%lx ", (unsigned long)requested_addr);

		flags |= MAP_FIXED;
	}

	// Map reserved/uncommitted pages PROT_NONE so we fail early if we
	// touch an uncommitted page. Otherwise, the read/write might
	// succeed if we have enough swap space to back the physical page.
	addr = (char*)mmap(requested_addr, bytes, PROT_NONE,
											 flags, -1, 0);

	return addr == MAP_FAILED ? NULL : addr;
}



/**
 * Commit memory at reserved memory range.
 *  
 */
char* commit_anon_memory(char* start_addr, unsigned long size, bool exec) {
	int prot = (exec == true) ? PROT_READ|PROT_WRITE|PROT_EXEC : PROT_READ|PROT_WRITE;

  // MAP_FIXED will override the old mapping
	unsigned long res = (unsigned long)mmap(start_addr, size, prot,
									 MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0);
	
	// commit memory successfully.
	if (res == (unsigned long) MAP_FAILED) {

		// print errno here.
		return NULL;
	}

	return start_addr;
}

int main() {
  int ret = 0;
  unsigned long i;
  unsigned long request_addr = 0x400000000000UL;
  unsigned long size = 0x40000000;  // byte size, 1GB data
  int* user_buf;

  // #1 allocate the virtual range of the buffer
  user_buf = (int*)reserve_anon_memory((char*)request_addr, size, true);
  if (user_buf == NULL) {
    printf("Reserve user_buffer, 0x%lx failed. \n",
           (unsigned long)request_addr);
  } else {
    printf("Reserve user_buffer: 0x%lx, bytes_len: 0x%lx \n",
           (unsigned long)user_buf, size);
  }

  // 1.2) commit the space
  user_buf = (int*)commit_anon_memory((char*)request_addr, size, false);
  if (user_buf == NULL) {
    printf("Commit user_buffer, 0x%lx failed. \n", (unsigned long)request_addr);
  } else {
    printf("Commit user_buffer: 0x%lx, bytes_len: 0x%lx \n",
           (unsigned long)user_buf, size);
  }

	// 1.3) touch the user space
	// memset(user_buf, -1, size);
	// for (i = 0; i < size/sizeof(int); i += 16*M)
  //   	printf("#1.3 user_buf[%lu]: %d\n", i, user_buf[i]);


  // #2 Ask kernel to fill the physical pages of the buffer
  ret = syscall(SYS_USER_KERNEL_BUF, (unsigned long)user_buf, size);
  if (ret) {
    printf("syscall error, with code %d\n", ret);
    goto out;
  }

  // #3 Check the value of the allcoated data
  // Should be the debug value, 0x2020202 (Enable debug for kernel)
  for (i = 0; i < size/sizeof(int); i += 16*M)
    printf("#3 user_buf[%lu]: 0x%x\n", i, user_buf[i]);


  // #4 Try to re-write the value of the buffer
  for (i = 0; i < size/sizeof(int); i += 16*M){
    user_buf[i] = 1;
    printf("#4 user_buf[%lu]: 0x%x\n",i, user_buf[i]);
  }




out:
  return 0;
}
