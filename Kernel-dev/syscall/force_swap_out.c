/**
 * @file force_swap_out.c
 * @author your name (you@domain.com)
 * @brief Force the kernel to swap out a range of virtual address space to swap partition.
 * @version 0.1
 * @date 2021-10-16
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>  
#include <sys/errno.h>
#include <unistd.h>


#include "stdint.h"
#include "stdio.h"


// Define a name for syscall do_semeru_rdma_ops
#define SYS_do_semeru_rdma_ops  333 

typedef enum {true, false} bool;

extern errno;




/**
 * Reserve memory at fixed address 
 */
static char* reserve_anon_memory(char* requested_addr, size_t bytes, bool fixed) {
	char * addr;
	int flags;

	flags = MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS;   
	if (fixed == true) {
			printf("Request fixed addr 0x%lx ", (size_t)requested_addr);

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
char* commit_anon_memory(char* start_addr, size_t size, bool exec) {
	int prot = (exec == true) ? PROT_READ|PROT_WRITE|PROT_EXEC : PROT_READ|PROT_WRITE;
	size_t res = (size_t)mmap(start_addr, size, prot,
																		 MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0);   // MAP_FIXED will override the old mapping
	
	// commit memory successfully.
	if (res == (size_t) MAP_FAILED) {

		// print errno here.
		return NULL;
	}

	return start_addr;
}




int main(){
				
	int type = 0x1;		// type 0x1 write, 0x2 read
	int target_server = 0x0;   // memory server id. e.g. 0, 1
	size_t request_addr 	= 0x400100000000; // start of RDMA data space
	//size_t size  					=	0x2000;		// 8KB, have to confirm  the physical memory are contiguous, if the large than 16KB, use huge page.
	//size_t size  					=	0x40000000; // 64(30+30+4) pages, test scatter-gather design.
	size_t size  					=	0x200000; // 2MB, a single PMD entry
	char* user_buff;
	size_t i;
	size_t initial_val		= 999;  //
	int syscall_ret 				= 0;

	// 1) reserve space by mmap
	user_buff = reserve_anon_memory((char*)request_addr, size, true );
	if(user_buff == NULL){
		printf("Reserve user_buffer, 0x%lx failed. \n", (size_t)request_addr);
	}else{
		printf("Reserve user_buffer: 0x%lx, bytes_len: 0x%lx \n",(size_t)user_buff, size);
	}

	// 2) commit the space
	user_buff = commit_anon_memory((char*)request_addr, size, false);
	if(user_buff == NULL){
		printf("Commit user_buffer, 0x%lx failed. \n", (size_t)request_addr);
	}else{
		printf("Commit user_buffer: 0x%lx, bytes_len: 0x%lx \n",(size_t)user_buff, size);
	}

	// initialize the user_space buffer
	size_t * buf_ptr = (size_t*)user_buff;
	for(i=0; i< size/sizeof(size_t); i++ ){
		buf_ptr[i] = initial_val;  // the max value.
	}



	// 1) RDMA write
	//
	type = 0x4;
	printf("Force swap out [0x%lx, 0x%lx]\n", request_addr, request_addr + size );
  syscall_ret = syscall(SYS_do_semeru_rdma_ops,type, target_server, user_buff, size);
  printf("System call id SYS_do_semeru_rdma_ops, type 0x%x, memory server %d, returned %d \n", type, target_server, syscall_ret);


	//sleep(5);


	printf("After force swap-out, read the first size_t of the user_buffer: 0x%lx \n",*(size_t*)user_buff);

	return 0;
}
