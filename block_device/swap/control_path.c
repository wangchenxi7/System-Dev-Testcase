
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>  
#include <sys/errno.h>
#include <unistd.h>


#include "stdint.h"
#include "stdio.h"

// Get the macro values in this header
//#include <linux/swap_global_struct.h>
#define NUM_OF_MEMORY_SERVER 2


// Define a name for syscall do_semeru_rdma_ops
#define SYS_do_semeru_rdma_ops  333 

typedef enum {true, false} bool;

extern errno;




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
	unsigned long res = (unsigned long)mmap(start_addr, size, prot,
																		 MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0);   // MAP_FIXED will override the old mapping
	
	// commit memory successfully.
	if (res == (unsigned long) MAP_FAILED) {

		// print errno here.
		return NULL;
	}

	return start_addr;
}




int main(){
				
	int type = 0x1;
	unsigned long request_addr 	= 0x400080000000;
	//unsigned long size  			=	0x1e000;		// 8KB, have to confirm  the physical memory are contiguous, if the large than 16KB, use huge page.
	unsigned long size  				=	0x40000000; // 64(30+30+4) pages, test scatter-gather design.
	char* user_buff;
	unsigned long i;
	int mem_server_id;
	unsigned long initial_val		= -1;  // 0xffff ... ff
	int syscall_ret 						= 0;

	// 1) reserve space by mmap
	user_buff = reserve_anon_memory((char*)request_addr, size, true );
	if(user_buff == NULL){
		printf("Reserve user_buffer, 0x%lx failed. \n", (unsigned long)request_addr);
	}else{
		printf("Reserve user_buffer: 0x%lx, bytes_len: 0x%lx \n",(unsigned long)user_buff, size);
	}

	// 2) commit the space
	user_buff = commit_anon_memory((char*)request_addr, size, false);
	if(user_buff == NULL){
		printf("Commit user_buffer, 0x%lx failed. \n", (unsigned long)request_addr);
	}else{
		printf("Commit user_buffer: 0x%lx, bytes_len: 0x%lx \n",(unsigned long)user_buff, size);
	}

	// initialize the user_space buffer
	unsigned long * buf_ptr = (unsigned long*)user_buff;
	for(i=0; i< size/sizeof(unsigned long); i++ ){
		buf_ptr[i] = initial_val;  // the max value.
	}



	// 1) RDMA write
	//
	type = 0x2;
	for(mem_server_id = 0; mem_server_id< NUM_OF_MEMORY_SERVER; mem_server_id++){

		printf("Before syscall - memory server[%d] RDMA Write, first unsigned long of the user_buffer: 0x%lx \n",
			mem_server_id, *(unsigned long*)user_buff);
  	syscall_ret = syscall(SYS_do_semeru_rdma_ops, type, mem_server_id, user_buff, size);
  	printf("Invoke#1, System call id SYS_do_semeru_rdma_ops, type 0x%x , memory server[%d], returned %d \n", 
			type, mem_server_id, syscall_ret);

		printf(" memory server[%d]	Pressure test, write the data again.\n", mem_server_id);
		syscall_ret = syscall(SYS_do_semeru_rdma_ops,type, mem_server_id, user_buff, size);
  	printf("Invoke#2, System call id SYS_do_semeru_rdma_ops, type 0x%x, memory server[%d], returned %d \n", 
			type, mem_server_id, syscall_ret);
	}

	//sleep(5);



  // 2) RDMA read
  //
	type =0x1;

	for(mem_server_id = 0; mem_server_id< NUM_OF_MEMORY_SERVER; mem_server_id++){
		// reset the value to test read/write 
		printf("reset the value on CPU server to 0. \n");
		for(i=0; i< size/sizeof(unsigned long); i++ ){
			buf_ptr[i] = 0;  // the max value.
		}

		printf("Before syscall - memory server[%d] RDMA read, first unsigned long of the user_buffer: 0x%lx \n",
			mem_server_id, *(unsigned long*)user_buff);

  	syscall_ret = syscall(SYS_do_semeru_rdma_ops, type, mem_server_id, user_buff, size);
  	printf("System call id SYS_do_semeru_rdma_ops, type 0x%x, memory server[%d], returned %d \n", 
			type, mem_server_id, syscall_ret);
  	
		sleep(3);

	// busy checking the first unsigned long value of the buffer.
	// give rdma some time to run
	// for(i=0; i<initial_val; i++){  // inifinit
	// 	if(*(unsigned long*)user_buff != initial_val ){
	// 		printf("1-sided RDMA read finished. value 0x%lx \n", *(unsigned long*)user_buff);
	// 		break;
	// 	}else{
	// 		//keep busy waiting 
	// 		//if(i%8 == 0)
	// 			printf("busy waiting, time counter: 0x%lx \n",i);
	// 	}


	// }

		printf("After syscall RDMA read,  memory server[%d], FIRST unsigned long of the user_buffer: 0x%lx \n",
			mem_server_id, *(unsigned long*)user_buff);
		printf("After syscall RDMA read, memory server[%d], LAST unsigned long of the user_buffer: 0x%lx \n",
			mem_server_id, *(unsigned long*)(user_buff +  size/sizeof(unsigned long) - 1) );

	}

	return 0;
}
