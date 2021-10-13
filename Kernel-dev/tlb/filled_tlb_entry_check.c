/**
 * @file filled_tlb_entry_check.c
 * @author your name (you@domain.com)
 * @brief We intercept the page fault and insert our own TLB entires.
 * 		This testcase is to tset if we successfully inserted the fake PTE into TLB buffer.
 * 		We trigger anonymous page fault on specific virtual range.
 * 		And then, let kernel always insert its previous PTE into the TLB buffer.
 * 		As a result, this app expects getting the previos page's result back instead of its own page.
 * @version 0.1
 * @date 2021-10-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>  
#include <sys/errno.h>


#include "stdint.h"
#include "stdio.h"



typedef enum {true, false} bool;

extern errno;

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif


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
	unsigned long request_addr 	= 0x400100000000; //Start at Data Region
	//unsigned long request_addr 	= 0x400000000000; //  Start at Meta Region
	//unsigned long size  		= 0x400000000;	// 16GB,
	//unsigned long size  		= 0x80000000;	// 2GB, unsigned long array
	//unsigned long size  		= 0x40000000; // 1GB
	//unsigned long size  		= 0x3000000;	// 48MB, for unsigned long, length is 0x4,000,000
	unsigned long size  		= 0x100000;	// 1MB, 256 pages


	char* user_buff;
	unsigned long i,j;
	unsigned long sum = 0;

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

	printf("Phase #1, Initialize the content of each Page. \n");
	unsigned long * buf_ptr = (unsigned long*)user_buff;
	for(j=0; j< size/sizeof(unsigned long); j+=(PAGE_SIZE/sizeof(unsigned long)) ){

		// read the initial value before wrting
		printf("before write: page[%lu], unsigned long offset[%d] : %lu\n",
			j/(PAGE_SIZE/sizeof(unsigned long)), 0,	buf_ptr[j+0]);

		for(i= 0; i<(PAGE_SIZE/sizeof(unsigned long)) ; i++ ){
			// Initialize the content of the page to the page num.
			// Trigger a page fault here.
			// The kernel change the pte value to the its previous physical page.
			// Skip the first page.
			buf_ptr[j+i] = j/(PAGE_SIZE/sizeof(unsigned long));  
		}

		// print the value of the initialized page
		// As a result, we are expecting the data as 
		// page[0], unsigned long offset[0] : 0
		// page[0], unsigned long offset[0] : 1
		// page[1], unsigned long offset[0] : 1
		// page[1], unsigned long offset[0] : 2
		// page[2], unsigned long offset[0] : 2
		// ...
		if(j >= PAGE_SIZE/sizeof(unsigned long)){
			// previous page
			// We overwrite the data on previous page
			printf("Prev Page[%lu], unsigned long offset[%d] : %lu \n",
				j/(PAGE_SIZE/sizeof(unsigned long)) -1 , 0, buf_ptr[j - (PAGE_SIZE/sizeof(unsigned long)) +0]);
		}
		
		// current page
		printf("Page[%lu], unsigned long offset[%d] : %lu \n\n",
				j/(PAGE_SIZE/sizeof(unsigned long)), 0, buf_ptr[j+0]);
		
	}

	sum =0;
	printf("Phase #2, avoid optimizing. Get the sum of the data\n");
	for(i=0; i< size/sizeof(unsigned long); i++ ){
		sum +=buf_ptr[i];  // the sum should be 0x7,FFF,FFE,000,000.
	}

	printf("sum : 0x%lx \n",sum);

	return 0;
}
