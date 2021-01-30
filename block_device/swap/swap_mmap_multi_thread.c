
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



typedef enum {true, false} bool;

extern errno;

//#define ARRAY_BYTE_SIZE 0xc0000000UL

#define ARRAY_START_ADDR	0x400000000000UL		
#define ARRAY_BYTE_SIZE 	0x80000000UL  // 2GB

int online_cores = 16;

struct thread_args{
	size_t	thread_id; // thread index, start from 0
	char* 	user_buf;
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
	char* user_buff	=	(char*)args->user_buf;
	size_t array_slice = ARRAY_BYTE_SIZE/sizeof(unsigned long)/online_cores;
	size_t array_start = array_slice * tid;
	size_t i, sum;

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


// pthread function #1
// scan the entire array from start to the end.
// make the memory acces random
void *scan_array_overleap(void* _args){
	struct thread_args *args = (struct thread_args*)_args;
	size_t tid = (size_t)args->thread_id; // 0 to online_cores
	char* user_buff	=	(char*)args->user_buf;
	size_t array_slice = ARRAY_BYTE_SIZE/sizeof(unsigned long); // the entire array
	size_t array_start = 0; 	// scan from start
	size_t i, sum;


	//
	// ！！ Fix me !!

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
				
	int type = 0x1;
	unsigned long request_addr 	= ARRAY_START_ADDR; // start of RDMA meta space, 1GB not exceed the swap partitio size.
	unsigned long size  		= ARRAY_BYTE_SIZE;	// 512MB, for unsigned long, length is 0x4,000,000
	char* user_buff;
	unsigned long i;
	unsigned long sum = 0;
	pthread_t threads[online_cores];
	struct thread_args args;
	int ret =0;

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
