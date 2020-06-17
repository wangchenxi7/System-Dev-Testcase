/**
 * syscall mremap 163
 * 
 * 1) Extend current virtual memory range
 * 2) Remap the virtual memory range to another range of memory
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




int main(int argc, char* argv[]){

  char* target_buff;
	uint64_t target_request_addr 	= 0x40000000; // start of RDMA meta space, 1GB not exceed the swap partitio size.
	uint64_t target_size  					=	0x10000;		// 64KB, for uint64_t, length is 0x400

	char* user_buff;
  uint64_t user_buffer_addr = 0x20000000; // 512MB
  uint64_t user_buffer_size = 0x2000; // 8KB
	uint64_t i;
	uint64_t sum = 0;
	char* ret;
	uint64_t * buf_ptr;

	// 1) reserve space by mmap

  // Reserve but not touch.
	user_buff = commit_anon_memory((char*)user_buffer_addr, user_buffer_size, false );
  if(user_buff == NULL){
		printf("Reserve user_buffer, 0x%llx failed. \n", (uint64_t)user_buffer_addr);
	}else{
		printf("Reserve user_buffer: 0x%llx, bytes_len: 0x%llx \n",(uint64_t)user_buffer_addr, user_buffer_size);
	}

  // The virtual range with content.
  target_buff = commit_anon_memory((char*)target_request_addr, target_size, false );
	if(user_buff == NULL){
		printf("Commit target, 0x%llx failed. \n", (uint64_t)target_request_addr);
	}else{
		printf("Commit target: 0x%llx, bytes_len: 0x%llx \n",(uint64_t)target_request_addr, target_size);
	}

  // 2) Initialize the target buffer

	// 2.1) Initialize the  user buffer
	buf_ptr = (uint64_t*)user_buff;
	for(i=0; i< user_buffer_size/sizeof(uint64_t); i++ ){
		buf_ptr[i] = 7;  // a fixed value.
	}

  // check the initialized value
  printf("Check the initialized value of target buffer"); 
  buf_ptr =  (uint64_t*)user_buff;
  for(i=0; i< user_buffer_size/sizeof(uint64_t); i+=512  ){
    printf("user_buff[%d] %llu \n", i, buf_ptr[i]);
  }

	// 2.2) Initialize the  target buffer
	buf_ptr = (uint64_t*)target_buff;
	for(i=0; i< target_size/sizeof(uint64_t); i++ ){
		buf_ptr[i] = i;  // assigned to the index.
	}

  // check the initialized value
  printf("Check the initialized value of target buffer"); 
  buf_ptr =  (uint64_t*)target_buff;
  for(i=0; i< target_size/sizeof(uint64_t); i+=512  ){
    printf("target_buff[%d] %llu \n", i, buf_ptr[i]);
  }


  //3) Do the mremap
	// a. Copy the data from old buffer, user_buff, to the new new buffer, target_buff;
	// b. unmap the old buffer, user_buff. Access it will cause segmentation fault.
	// c. unmap the new buffer, target_buff. Accessing its content is safe.
	int prot = MREMAP_FIXED | MREMAP_MAYMOVE;
  ret = mremap(user_buff, user_buffer_size, target_size, prot, target_buff);
	if(ret == MAP_FAILED ){ // MAP_FAILED  is (void*)-1
		printf("mremap failed. with return %s \n", ret);
	}else{
		printf("mremap success, return 0x%llx \n", (uint64_t)ret );
		printf("user_buff start address 0x%llx \n", (uint64_t)user_buff );
		printf("target_buff start address 0x%llx \n", (uint64_t)target_buff );
	}


	// 3.1) check the remapped user_buffer value
  printf("Check the remapped value of returned buffer 0x%llx \n", (uint64_t)ret); 
  buf_ptr =  (uint64_t*)ret;
  for(i=0; i< target_size/sizeof(uint64_t); i+=512  ){
    printf("ret[%d] %llu \n", i, buf_ptr[i]);
  }

	// 3.2 Check the value of the new buffer.
	//     It's content will be override by the content of old buffer, user_buff.
	printf("Check the remapped value of target buffer 0x%llx \n", (uint64_t)target_buff ); 
  buf_ptr =  (uint64_t*)target_buff;
  for(i=0; i< target_size/sizeof(uint64_t); i+=512  ){
    printf("target_buff[%d] %llu \n", i, buf_ptr[i]);
  }

	// 3.3 Access the content of old buffer, user_buff will cause segmentation fault.
	//     It's unmapped.


  return 0;
}

