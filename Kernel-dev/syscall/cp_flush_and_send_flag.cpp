/**
 * @file cp_flush_and_sent_flag.c
 * @author your name (you@domain.com)
 * @brief Send a flag via the control path. 
 * 	Do not sent the flag until all the threads eixt thier swap zone.
 * @version 0.1
 * @date 2021-11-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */


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

// c++ stl multithread random number generator
#include <random>


/**
 * Warning : the tradition srand() is a sequential generator. 
 * 	The multiple threads will be bound at the random number generation.
 */

#define SYS_do_semeru_rdma_ops  333 

#define ONE_MB    1048576UL				// 1024 x 2014 bytes
#define ONE_GB    1073741824UL   	// 1024 x 1024 x 1024 bytes

#define PAGE_SIZE  4096
#define PAGE_SHIFT 12

//typedef enum {true, false} bool;

extern errno;

//#define ARRAY_BYTE_SIZE 0xc0000000UL

#define ARRAY_START_ADDR	0x400100000000UL //data region
//#define ARRAY_BYTE_SIZE   1*ONE_GB  // 
#define ARRAY_BYTE_SIZE   128*ONE_MB 


int online_cores = 8;

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
void *scan_array_random_overleap(void* _args){
	struct thread_args *args = (struct thread_args*)_args;
	size_t tid = (size_t)args->thread_id; // 0 to online_cores
	char* user_buff	=	(char*)args->user_buf;
	size_t array_slice = ARRAY_BYTE_SIZE/sizeof(unsigned long); // the entire array
	size_t array_start = 0; 	// scan from start
	size_t i, sum;

	thread_local std::mt19937 engine(std::random_device{}());
	std::uniform_int_distribution<int> dist(1, ONE_GB);

	printf("Thread[%lu] Phase #1, trigger swap out. \n",tid);
	unsigned long * buf_ptr = (unsigned long*)user_buff;
	for( i = array_start  ; i < array_start + array_slice  ; i++ ){
		buf_ptr[i] = i;  // the max value.
	}

	sum =0;
	printf("Thread[%lu] Phase #2, trigger swap in.\n", tid);
	for( i=array_start; i<  array_start + array_slice; i+=32){ // step is N
		unsigned long index_to_access = (unsigned long)dist(engine)*i % (array_slice/sizeof(unsigned long));

		//debug - check the random level
		//printf("Thread[%lu] access buf_ptr[%lu], page[%lu] \n", tid, index_to_access, (index_to_access >> (PAGE_SHIFT -3 ) ) );

		sum +=buf_ptr[index_to_access];  // the sum should be 0x7,FFF,FFE,000,000. 
	}

	printf("Thread[%lu] sum : 0x%lx of array_slice[0x%lx, 0x%lx ]  \n", tid, sum, (unsigned long)(buf_ptr + array_start), (unsigned long)(buf_ptr + array_slice)  );

	pthread_exit(NULL);
}



void *scan_array_sequential_overleap(void* _args){
	struct thread_args *args = (struct thread_args*)_args;
	size_t tid = (size_t)args->thread_id; // 0 to online_cores
	char* user_buff	=	(char*)args->user_buf;
	size_t array_slice = ARRAY_BYTE_SIZE/sizeof(unsigned long); // the entire array
	size_t array_start = 0; 	// scan from start
	size_t i, sum;

	thread_local std::mt19937 engine(std::random_device{}());
	std::uniform_int_distribution<int> dist(1, ONE_GB);

	printf("Thread[%lu] Phase #1, trigger swap out. \n",tid);
	unsigned long * buf_ptr = (unsigned long*)user_buff;
	for( i = array_start  ; i < array_start + array_slice  ; i++ ){
		buf_ptr[i] = i;  // the max value.
	}

	sum =0;
	printf("Thread[%lu] Phase #2, trigger swap in.\n", tid);
	for( i=array_start; i<  array_start + array_slice; i+=(PAGE_SIZE/sizeof(unsigned long)) ){ // step is PAGE
		sum +=buf_ptr[i];  // the sum should be 0x7,FFF,FFE,000,000.
	}

	printf("Thread[%lu] sum : 0x%lx of array_slice[0x%lx, 0x%lx ]  \n", tid, sum, (unsigned long)(buf_ptr + array_start), (unsigned long)(buf_ptr + array_slice)  );

	pthread_exit(NULL);
}


// force page swap-out
void *force_array_swapout_no_overleap(void* _args){
	struct thread_args *args = (struct thread_args*)_args;
	size_t tid = (size_t)args->thread_id; // 0 to online_cores
	char* user_buff	=	(char*)args->user_buf;
	size_t array_slice = ARRAY_BYTE_SIZE/sizeof(unsigned long)/online_cores;
	size_t array_start = array_slice * tid;
	size_t i, sum;
	int type;


	printf("Thread[%lu] Phase #1, trigger swap out. \n",tid);
	unsigned long * buf_ptr = (unsigned long*)user_buff;
	for( i = array_start  ; i < array_start + array_slice  ; i++ ){
		buf_ptr[i] = i;  // the max value.
	}


	printf("Thread[%lu] Phase#2, force swap out all the pages", tid);
	type = 0x4;
	int target_server = 0;
	size_t array_slice_byte = array_slice * sizeof(unsigned long);
	char* flush_start_byte_addr = user_buff + tid * array_slice_byte;

	printf("Force swap out [0x%lx, 0x%lx]\n", (size_t)flush_start_byte_addr, (size_t)flush_start_byte_addr + array_slice_byte );
  int syscall_ret = syscall(SYS_do_semeru_rdma_ops,type, target_server, flush_start_byte_addr, array_slice_byte );
	printf("System call id SYS_do_semeru_rdma_ops, type 0x%x, memory server %d, returned %d \n", type, target_server, syscall_ret);


	sum =0;
	printf("Thread[%lu] Phase #3, trigger swap in.\n", tid);
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
	struct thread_args args[online_cores]; // pass value by reference, so keep a copy for each thread
	int ret =0;
	int syscall_ret;
	int target_server;
	srand(time(NULL));		// generate a random interger

	//online_cores = sysconf(_SC_NPROCESSORS_ONLN);

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

	
	for(i=0; i< online_cores; i++){
		args[i].user_buf = user_buff;
		args[i].thread_id = i;
		
		ret = pthread_create(&threads[i], NULL, force_array_swapout_no_overleap, (void*)&args[i]);
		//ret = pthread_create(&threads[i], NULL, scan_array_sequential_overleap, (void*)&args[i]);
		if (ret){
      printf("ERROR; return code from pthread_create() is %d\n", ret);
      return 0;
    }

	}


	
	// Send the end flag via the flush-and-sent-flag path
	sleep(1);

	// type = 0x5;
	// target_server = 0;
	// printf("Sent the flag [0x%lx, 0x%lx]\n", request_addr, request_addr + PAGE_SIZE );
  // syscall_ret = syscall(SYS_do_semeru_rdma_ops,type, target_server, user_buff, PAGE_SIZE);
  // printf("System call id SYS_do_semeru_rdma_ops, type 0x%x, memory server %d, returned %d \n", type, target_server, syscall_ret);


	pthread_exit(NULL); // main thread return.



	return 0; //useless ?
}
