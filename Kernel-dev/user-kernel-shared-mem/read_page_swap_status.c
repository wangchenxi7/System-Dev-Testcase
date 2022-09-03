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


#ifndef K
  #define K 1024UL
#endif

#ifndef M
  #define M 1024*1024UL
#endif

typedef enum {true, false} bool;


/**
 * The structure shared between user and kernel.
 * 
 */


// page status 
enum page_stat{
  MAPPED   = 0,
  UNMAPPED = 1,
  SWAPPED  = 2,
};


// [TODO] Actually, we can store the present of not information
// into the page_stat and let the runtime query this information
// In this case, the epoch is useless?
// [TODO] Record the process ID. Right now only the data of
// single process can be swapped out.
//
//  Structure of the epoch_struct	
//  |--4 bytes for eppch --|-- 4 bytes for legnth --|-- unsigned char array --|
struct epoch_struct{
  unsigned int epoch;   // the first 32 bits for epoch recording
  unsigned int length;  // length of the page_stats
  //unsigned int page_stats[COVERED_MEM_LENGTH];  // the epoch value for each page
  unsigned char page_stats[];
};




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
  unsigned long user_kernel_shared_buf_start_addr = 0x100000000000UL;
  unsigned long user_kernel_shared_buf_size = 0x80000UL;  // byte size, 512KB data, ~ cover 2GB Java heap
  struct epoch_struct* user_buf;

  //the Java heap
  unsigned long semeru_start_addr = 0x400000000000UL;
  unsigned long semeru_heap_size  = 0x40000000UL;  // 1GB data size
  int* semeru_heap = NULL;
  unsigned long sum = 0;

  //
  // Allocate the user-kernel shared buffer 
  //
  // #1 allocate the virtual range of the buffer
  user_buf = (struct epoch_struct*)reserve_anon_memory((char*)user_kernel_shared_buf_start_addr, user_kernel_shared_buf_size, true);
  if (user_buf == NULL) {
    printf("Reserve user_buffer, 0x%lx failed. \n",
           (unsigned long)user_kernel_shared_buf_start_addr);
  } else {
    printf("Reserve user_buffer: 0x%lx, bytes_len: 0x%lx \n",
           (unsigned long)user_buf, user_kernel_shared_buf_size);
  }

  // 1.2) commit the space
  user_buf = (struct epoch_struct*)commit_anon_memory((char*)user_kernel_shared_buf_start_addr, user_kernel_shared_buf_size, false);
  if (user_buf == NULL) {
    printf("Commit user_buffer, 0x%lx failed. \n", (unsigned long)user_kernel_shared_buf_start_addr);
  } else {
    printf("Commit user_buffer: 0x%lx, bytes_len: 0x%lx \n",
           (unsigned long)user_buf, user_kernel_shared_buf_size);
  }

	// 1.3) touch the user space
	// memset(user_buf, -1, size);
	// for (i = 0; i < size/sizeof(int); i += 16*M)
  //   	printf("#1.3 user_buf[%lu]: %d\n", i, user_buf[i]);


  // #2 Ask kernel to fill the physical pages of the buffer
  ret = syscall(SYS_USER_KERNEL_BUF, (unsigned long)user_buf, user_kernel_shared_buf_size);
  if (ret) {
    printf("syscall error, with code %d\n", ret);
    goto out;
  }

  // #3 Check the value of the allcoated data
  printf("epoch %d \n",user_buf->epoch);
  printf("array length %x \n", user_buf->length);
 
  for(i=0; i<user_buf->length; i+=256*K ){
    printf("page_stat[%lu] %u \n", i, user_buf->page_stats[i]);
  }


  //
  // Allocate a fake Java heap
  //
   semeru_heap = (int*)reserve_anon_memory((char*)semeru_start_addr, semeru_heap_size, true);
  if (semeru_heap == NULL) {
    printf("Reserve semeru_heap, 0x%lx failed. \n",
           (unsigned long)semeru_start_addr);
  } else {
    printf("Reserve semeru_heap: 0x%lx, bytes_len: 0x%lx \n",
           (unsigned long)semeru_heap, semeru_heap_size);
  }

  // 1.2) commit the space
  semeru_heap = (int*)commit_anon_memory((char*)semeru_start_addr, semeru_heap_size, false);
  if (semeru_heap == NULL) {
    printf("Commit semeru_heap, 0x%lx failed. \n", (unsigned long)semeru_start_addr);
  } else {
    printf("Commit semeru_heap: 0x%lx, bytes_len: 0x%lx \n",
           (unsigned long)semeru_heap, semeru_heap_size);
  }


  // touch and set the semeru heap
  // trigger swap out
  for(i=0; i<semeru_heap_size/sizeof(int);  i++ ){
    semeru_heap[i] = i*i;
    sum+=semeru_heap[i];
  }

  printf("Sum is %lu\n", sum);


  //
  // check the page status
  //
  for(i=0; i<user_buf->length; i+=2*K ){
    printf("page_stat[%lu] %u \n", i, user_buf->page_stats[i]);
  }



out:
  return 0;
}
