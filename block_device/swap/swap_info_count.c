/**
 * Use madvice() MAD_FREE to put a specific range of memory into inactive list. 
 * These pages will be swapped out when there is a memory pressure.
 * 
 */

#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>  
#include <sys/errno.h>
#include </usr/include/asm-generic/mman-common.h>

#include "stdint.h"
#include "stdio.h"


// Semeru
//#include <linux/swap_global_struct.h>

#define MADV_FLUSH_RANGE_TO_REMOTE 20  // Should include from linux header

#define SYS_SWAP_STAT_RESET			335
#define SYS_ON_DEMAND_SWAPIN		337


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
	uint64_t request_addr 	= 0x40000000; // start of RDMA meta space, 1GB not exceed the swap partitio size.
	//uint64_t size  					=	0x100000;		// 1MB, for uint64_t, length is 0x20,000
	uint64_t size  					=	0x40000000;		// 1GB data
	char* user_buff;
	uint64_t i;
	uint64_t sum = 0;
	uint64_t on_demand_swapin = 0;
	int ret;

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

	printf("Phase#1, trigger swap-out \n");
	uint64_t * buf_ptr = (uint64_t*)user_buff;
	for(i=0; i< size/sizeof(uint64_t); i++ ){
		buf_ptr[i] = i;  // the max value.
	}

	printf("	Reset the swap out statistics monitoring array\n");
	ret = syscall(SYS_SWAP_STAT_RESET, request_addr, size);
	printf("	SYS_SWAP_STAT_RESET returned %d \n", ret);

	printf("	#1 Check current swapped out pages num\n");
	on_demand_swapin = syscall(SYS_ON_DEMAND_SWAPIN);
	printf("	#1 on-demand swap-in pages num 0x%llx \n", on_demand_swapin);  // should be 0.



	sum =0;
	printf("Phase#2, trigger swap-in, stride in 8 pages.\n");
	for(i=0; i< size/sizeof(uint64_t); i+=4096 ){ // access 1 u64 per 32KB, 8 pages.
		printf("buf_ptr[0x%llx] 0x%llx \n",(uint64_t)i, buf_ptr[i]);
		sum +=buf_ptr[i];  // the sum should be 0x7f0,000
	}

	printf("	# Check on-demand swap-in pages after re-access these pages\n");
	on_demand_swapin = syscall(SYS_ON_DEMAND_SWAPIN);
	printf("	# on-demand swap-in pages num %llu \n", on_demand_swapin);	// should be half the pages stay swapped out.

	// reset the information agian.
	printf("	Reset the swap out statistics monitoring array\n");
	ret = syscall(SYS_SWAP_STAT_RESET, request_addr, size);
	printf("	SYS_SWAP_STAT_RESET returned %d \n", ret);

	printf("Phase#3, trigger swap-in, streaming (access consecutive pages).\n");
	for(i=0; i< size/sizeof(uint64_t); i+=512 ){ // access 1 u64 per page, 8 pages.
		printf("buf_ptr[0x%llx] 0x%llx \n",(uint64_t)i, buf_ptr[i]);
		sum +=buf_ptr[i];  // the sum should be 0x7f0,000
	}

	printf("	# Check on-demand swap-in pages after re-access these pages\n");
	on_demand_swapin = syscall(SYS_ON_DEMAND_SWAPIN);
	printf("	# on-demand swap-in pages num %llu \n", on_demand_swapin);


	printf("sum : 0x%llx \n",sum);

	return 0;
}
