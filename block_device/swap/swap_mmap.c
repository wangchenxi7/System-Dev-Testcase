
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>  
#include <sys/errno.h>


#include "stdint.h"
#include "stdio.h"



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
	//uint64_t request_addr 	= 0x400100000000; //Start at Data Region
	uint64_t request_addr 	= 0x400000000000; //  Start at Meta Region
	uint64_t size  		= 0x200000000;	// 512MB, for uint64_t, length is 0x4,000,000
	//uint64_t size  		= 0x3000000;	// 48MB, for uint64_t, length is 0x4,000,000
	char* user_buff;
	uint64_t i;
	uint64_t sum = 0;

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

	printf("Phase #1, trigger swap out. \n");
	uint64_t * buf_ptr = (uint64_t*)user_buff;
	for(i=0; i< size/sizeof(uint64_t); i++ ){
		buf_ptr[i] = i;  // the max value.
	}

	sum =0;
	printf("Phase #2, trigger swap in.\n");
	for(i=0; i< size/sizeof(uint64_t); i++ ){
		sum +=buf_ptr[i];  // the sum should be 0x7,FFF,FFE,000,000.
	}

	printf("sum : 0x%llx \n",sum);

	return 0;
}
