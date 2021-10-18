
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
	size_t request_addr 	= 0x400100000000; // start of RDMA data space. Do NOT write random things into meta space
	//size_t size  					=	0x2000;		// 8KB, have to confirm  the physical memory are contiguous, if the large than 16KB, use huge page.
	size_t size  					=	0x40000000; // 1GB, test scatter-gather design.
	char* user_buff;
	size_t i;
	size_t initial_val		= -1;  //
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
	type = 0x2;
	printf("Before syscall - RDMA Write, first size_t of the user_buffer: 0x%lx \n",*(size_t*)user_buff);
  syscall_ret = syscall(SYS_do_semeru_rdma_ops,type, target_server, user_buff, size);
  printf("System call id SYS_do_semeru_rdma_ops, type 0x%x, memory server %d, returned %d \n", type, target_server, syscall_ret);



	//sleep(5);



  // 2) RDMA read
  //
	type =0x1;
	// reset the value to test read/write 
	printf("reset the value on current server to 0. \n");
	for(i=0; i< size/sizeof(size_t); i++ ){
		buf_ptr[i] = 0;  // the max value.
	}


	printf("Before syscall - RDMA read, first size_t of the user_buffer: 0x%lx \n",*(size_t*)user_buff);

  syscall_ret = syscall(SYS_do_semeru_rdma_ops, type, target_server, user_buff, size);
  printf("System call id SYS_do_semeru_rdma_ops, type 0x%x , memory server %d returned %d \n", type,  target_server, syscall_ret);
  
	sleep(3);


	// busy checking the first size_t value of the buffer.
	// give rdma some time to run
	// for(i=0; i<initial_val; i++){  // inifinit
	// 	if(*(size_t*)user_buff != initial_val ){
	// 		printf("1-sided RDMA read finished. value 0x%lx \n", *(size_t*)user_buff);
	// 		break;
	// 	}else{
	// 		//keep busy waiting 
	// 		//if(i%8 == 0)
	// 			printf("busy waiting, time counter: 0x%lx \n",i);
	// 	}


	// }

	printf("After syscall RDMA read, first size_t of the user_buffer: 0x%lx \n",*(size_t*)user_buff);

	return 0;
}
