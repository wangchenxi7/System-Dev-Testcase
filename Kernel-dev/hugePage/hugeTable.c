/**
 * Allocate huge page at 2MB.
 * 
 * 
 * Preparation 
 * 
 * 1) Confirm kernel .config with ?
 * 2) Enable huge page allocation by , echo 20 > /proc/sys/vm/nr_hugepages
 *    check by cat /proc/meminfo
 * 3) Use mmap with MAP_HUGETLB
 * 
 */  

#include <sys/mman.h>
#include <linux/kernel.h>
#include <sys/syscall.h>

#include "stdint.h"
#include "stdio.h"


// Define a name for syscall do_semeru_rdma_ops
#define SYS_do_semeru_rdma_ops  333 

typedef enum {true, false} bool;

extern errno;


/**
 * Reserve memory at fixed address 
 */
static char* reserve_anon_memory(char* requested_addr, uint64_t bytes, bool fixed) {
	char * addr;
	int flags;

	flags = MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS | MAP_HUGETLB;    // Huge page
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
	int flags = MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS| MAP_HUGETLB;
	uint64_t res = (uint64_t)mmap(start_addr, size, prot,	flags , -1, 0);   // MAP_FIXED will override the old mapping
	
	// commit memory successfully.
	if (res == (uint64_t) MAP_FAILED) {

		// print errno here.
		return NULL;
	}

	return start_addr;
}



int main(int argc, char* argv[]){

	int type = 0x1;
	uint64_t request_addr 	= 0x400000000000; // start of RDMA meta space
	uint64_t size  					=	0x400000;				// 4MB, use 2MB huge page.
	char* user_buff;
	uint64_t i;
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

	printf("First uint64_t is 0x%llx \n", *( (uint64_t*)buf_ptr ));
	
	printf("Please goto check the hugePage information by checking /proc/meminfo. \n");
	sleep(30);

	printf("Returned.\n");
	return 0;
}