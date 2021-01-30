#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>  
#include <sys/errno.h>
#include <pthread.h>

#include "stdint.h"
#include "stdio.h"
#include "time.h"
#include "stdlib.h"


#define THREAD_NUM		2
#define ARRAY_BYTE_SIZE 0x80000000  // 2GB

typedef enum {true, false} bool;

extern errno;


struct thread_args{
	size_t	thread_id; // thread index, start from 0
	char* 	user_buf_fast_path;
	char*		user_buf_slow_path;
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
	unsigned long res = (unsigned long)mmap(start_addr, size, prot,
																		 MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0);   // MAP_FIXED will override the old mapping
	
	// commit memory successfully.
	if (res == (unsigned long) MAP_FAILED) {

		// print errno here.
		return NULL;
	}

	return start_addr;
}




// pthread function #1
void *scan_array_no_overleap(void* _args){
	struct thread_args *args = (struct thread_args*)_args;
	size_t tid = (size_t)args->thread_id; // 0 to online_cores
	char*	 user_buff = NULL;
	size_t array_slice = 0;
	size_t array_start = array_slice * tid;
	size_t i, sum;

	if(tid == 0){
		// traverse the array for fastpath
		printf("Thread[%lu] go through fast path \n", tid);
		user_buff	=	(char*)args->user_buf_fast_path;
		array_slice = ARRAY_BYTE_SIZE/sizeof(unsigned long);
		array_start = 0;  // scan the array from start

	}else if(tid == 1){
		// traverse the array for slowpath
		printf("Thread[%lu] go through slow path \n", tid);
		user_buff	=	(char*)args->user_buf_slow_path;
		array_slice = ARRAY_BYTE_SIZE/sizeof(unsigned long);
		array_start = 0;  // scan the array from start
	}else{
		// wrong tid
		printf("tid %lu is wrong.\n", tid);
	}



	printf("Thread[%lu] Phase #1, trigger swap out. \n",tid);
	unsigned long * buf_ptr = (unsigned long*)user_buff;
	for( i = array_start  ; i < array_start + array_slice  ; i++ ){
		buf_ptr[i] = i;  // the max value.
	}

	sum =0;
	printf("Thread[%lu] Phase #2, trigger swap in.\n", tid);
	for( i=array_start; i<  array_start + array_slice; i++ ){
		sum +=buf_ptr[i];  // the sum should be 0x7,FFF,FFE,000,000.
	}

	printf("Thread[%lu] sum : 0x%lx \n", tid, sum);

	pthread_exit(NULL);
}




int main(){
				
	pthread_t threads[THREAD_NUM];
	struct thread_args args[THREAD_NUM];  // parameters are passed in reference parameter
	int ret = 0;

	//unsigned long request_addr 	= 0x400100000000; //Start at Data Region
	unsigned long request_addr_fast_path 	= 0x400000000000; //  Start at Meta Region
	unsigned long size_fast_path  		= ARRAY_BYTE_SIZE;	// 2GB, unsigned long array

	unsigned long request_addr_slow_path 	= 0x300000000000;	// not in reserved virtual address range
	unsigned long size_slow_path  		= ARRAY_BYTE_SIZE;		// 2GB

	//unsigned long size  		= 0x3000000;	// 48MB, for unsigned long, length is 0x4,000,000
	char* user_buff, *user_buff_slow_path;
	unsigned long i;
	unsigned long sum = 0;

	// 1) reserve space by mmap
	
	// 1.1 fastpath
	user_buff = reserve_anon_memory((char*)request_addr_fast_path, size_fast_path, true );
	if(user_buff == NULL){
		printf("Reserve user_buffer, 0x%lx failed. \n", (unsigned long)request_addr_fast_path);
	}else{
		printf("Reserve user_buffer: 0x%lx, bytes_len: 0x%lx \n",(unsigned long)user_buff, size_fast_path);
	}

	// 1.2 slow path
	user_buff_slow_path = reserve_anon_memory((char*)request_addr_slow_path, size_slow_path, true );
	if(user_buff_slow_path == NULL){
		printf("Reserve user_buffer, 0x%lx failed. \n", (unsigned long)request_addr_slow_path);
	}else{
		printf("Reserve user_buffer: 0x%lx, bytes_len: 0x%lx \n",(unsigned long)user_buff_slow_path, size_slow_path);
	}



	// 2) commit the space
	
	// 2.1) fastpath
	user_buff = commit_anon_memory((char*)request_addr_fast_path, size_fast_path, false);
	if(user_buff == NULL){
		printf("Commit user_buffer, 0x%lx failed. \n", (unsigned long)request_addr_fast_path);
	}else{
		printf("Commit user_buffer: 0x%lx, bytes_len: 0x%lx \n",(unsigned long)user_buff, size_fast_path);
	}


	// 2.2) slowpath
	user_buff_slow_path = commit_anon_memory((char*)request_addr_slow_path, size_slow_path, false);
	if(user_buff == NULL){
		printf("Commit user_buffer, 0x%lx failed. \n", (unsigned long)request_addr_slow_path);
	}else{
		printf("Commit user_buffer: 0x%lx, bytes_len: 0x%lx \n",(unsigned long)user_buff_slow_path, size_slow_path);
	}



	for(i=0; i< THREAD_NUM; i++){

		args[i].user_buf_fast_path = user_buff;
		args[i].user_buf_slow_path = user_buff_slow_path;
		args[i].thread_id = i;

		ret = pthread_create(&threads[i], NULL, scan_array_no_overleap, (void*)&args[i]);
		if (ret){
      printf("ERROR; return code from pthread_create() is %d\n", ret);
      return 0;
    }

	}


	pthread_exit(NULL); // main thread return.

	return 0;
}
