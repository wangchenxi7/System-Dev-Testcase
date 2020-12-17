
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>  
#include <sys/errno.h>
#include <pthread.h>

#include "stdint.h"
#include "stdio.h"



typedef enum {true, false} bool;

extern errno;

#define ARRAY_BYTE_SIZE 0xc0000000UL
int online_cores = 16;

struct thread_args{
	size_t	thread_id; // thread index, start from 0
	char* 	user_buf;
};


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


// pthread function
void *scan_array(void* _args){
	struct thread_args *args = (struct thread_args*)_args;
	size_t tid = (size_t)args->thread_id; // 0 to online_cores
	char* user_buff	=	(char*)args->user_buf;
	size_t array_slice = ARRAY_BYTE_SIZE/sizeof(uint64_t)/online_cores;
	size_t array_start = array_slice * tid;
	size_t i, sum;

	printf("Thread[%lu] Phase #1, trigger swap out. \n",tid);
	uint64_t * buf_ptr = (uint64_t*)user_buff;
	for( i = array_start  ; i < array_start + array_slice  ; i++ ){
		buf_ptr[i] = i;  // the max value.
	}

	sum =0;
	printf("Thread[%lu] Phase #2, trigger swap in.\n", tid);
	for( i=array_start; i<  array_start + array_slice; i++ ){
		sum +=buf_ptr[i];  // the sum should be 0x7,FFF,FFE,000,000.
	}

	printf("Thread[%lu] sum : 0x%llx \n", tid, sum);

	pthread_exit(NULL);
}



int main(){
				
	int type = 0x1;
	uint64_t request_addr 	= 0x400100000000; // start of RDMA meta space, 1GB not exceed the swap partitio size.
	uint64_t size  		= ARRAY_BYTE_SIZE;	// 512MB, for uint64_t, length is 0x4,000,000
	char* user_buff;
	uint64_t i;
	uint64_t sum = 0;
	pthread_t threads[online_cores];
	struct thread_args args;
	int ret =0;

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

	args.user_buf = user_buff;
	for(i=0; i< online_cores; i++){
		args.thread_id = i;
		ret = pthread_create(&threads[i], NULL, scan_array, (void*)&args);
		if (ret){
      printf("ERROR; return code from pthread_create() is %d\n", ret);
      return 0;
    }

	}


	pthread_exit(NULL); // main thread return.

	return 0; //useless ?
}
