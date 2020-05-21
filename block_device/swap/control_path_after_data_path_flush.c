/**
 * In some cases, the pte's corresponding page is swapped out.
 * The control path flush should detect this situation and skip these swapped out pages.
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


// Define names for syscall id
//

// Data path flush
#define MADV_FLUSH_RANGE_TO_REMOTE 20  // flush pages via swap and block layer 

// Control path flush
#define SYS_do_semeru_rdma_ops  333  // RDMA read 0x1, RDMA write0x2 

// monitor swap out ratio
#define SYS_SWAP_STAT_RESET			335
#define SYS_NUM_SWAP_OUT_PAGES	336

typedef enum {true, false} bool;

extern errno;




/**
 * Reserve memory at fixed address 
 */
static char* reserve_anon_memory(char* requested_addr, uint64_t bytes, bool fixed) {
	char * addr;
	int flags;

	flags = MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS;   
	if (fixed == true) {
			printf("Request fixed addr 0x%llx ", (uint64_t)requested_addr);

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
char* commit_anon_memory(char* start_addr, uint64_t size, bool exec) {
	int prot = (exec == true) ? PROT_READ|PROT_WRITE|PROT_EXEC : PROT_READ|PROT_WRITE;
	uint64_t res = (uint64_t)mmap(start_addr, size, prot,
																		 MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0);   // MAP_FIXED will override the old mapping
	
	// commit memory successfully.
	if (res == (uint64_t) MAP_FAILED) {

		// print errno here.
		return NULL;
	}

	return start_addr;
}




int main(){
				
	int type = 0x1;
	uint64_t request_addr 	= 0x400100000000; // [ 0x400,100,000,000 to 0x400,900,000,000) is RDMA Data space range. Only Data space support data-path flush.
	//uint64_t size  					=	0x1e000;		// 8KB, have to confirm  the physical memory are contiguous, if the large than 16KB, use huge page.
	uint64_t size  				=	0x14000; // 64(30+30+4) pages, test scatter-gather design.
	char* user_buff = NULL;
	uint64_t i;
	uint64_t swapped_out_pages = 0;
	uint64_t initial_val		= -1;  //
	int syscall_ret 				= 0;

	// 1) reserve space by mmap
	user_buff = reserve_anon_memory((char*)request_addr, size, true );
	if(user_buff == NULL){
		printf("Reserve user_buffer, 0x%llx failed. \n", (uint64_t)request_addr);
	}else{
		printf("Reserve user_buffer: 0x%llx, bytes_len: 0x%llx \n",(uint64_t)user_buff, size);
	}

	// 2) commit the space
	user_buff = commit_anon_memory((char*)request_addr, size, false);
	if(user_buff == NULL){
		printf("Commit user_buffer, 0x%llx failed. \n", (uint64_t)request_addr);
	}else{
		printf("Commit user_buffer: 0x%llx, bytes_len: 0x%llx \n",(uint64_t)user_buff, size);
	}

	// initialize the user_space buffer
	uint64_t * buf_ptr = (uint64_t*)user_buff;
	for(i=0; i< size/sizeof(uint64_t); i++ ){
		buf_ptr[i] = initial_val;  // the max value.
	}


	// 1) Reset the swap out monitor 
	//
	printf("Phase#1	Reset the swap out statistics monitoring array\n");
	syscall_ret = syscall(SYS_SWAP_STAT_RESET, request_addr, size);
	printf("	SYS_SWAP_STAT_RESET returned %d \n", syscall_ret);

	printf("	Check current swapped out pages num\n");
	swapped_out_pages = syscall(SYS_NUM_SWAP_OUT_PAGES, request_addr, size);
		printf(" [0x%llx, 0x%llx)	swapped out pages num 0x%llx \n", request_addr, request_addr + size, swapped_out_pages);  // should be 0.

	sleep(2);

	// 2) Data path flush(write)
	//
	printf("Phase#2, invoke madvice to flush 16 pages to swap partition \n");
	syscall_ret = madvise(user_buff, 0x10000, MADV_FLUSH_RANGE_TO_REMOTE);
	if(syscall_ret !=0){
		printf("MAD_FREE failed, return value %d \n", syscall_ret);
	}

	sleep(2);

	// check the swap out ratio agian.
	swapped_out_pages = syscall(SYS_NUM_SWAP_OUT_PAGES, request_addr, size);
	printf(" [0x%llx, 0x%llx)	swapped out pages num 0x%llx \n", request_addr, request_addr + size, swapped_out_pages);


	// 3) Control path flush
	//
	type = 0x2;  //write
	printf("Phase#3, Control path RDMA Write. Only half of the pages can be sent by control path.\n");
  syscall_ret = syscall(SYS_do_semeru_rdma_ops,type, user_buff, size);
  printf(" System call id SYS_do_semeru_rdma_ops, type 0x%x returned %d \n", type, syscall_ret);


	sleep(2);

	// 4) This may cause swap in.
	// printf("After syscall RDMA read, FIRST uint64_t of the user_buffer: 0x%llx \n",*(uint64_t*)user_buff);
	// printf("After syscall RDMA read, LAST uint64_t of the user_buffer: 0x%llx \n",*(uint64_t*)(user_buff +  size/sizeof(uint64_t) - 1) );

	return 0;
}
