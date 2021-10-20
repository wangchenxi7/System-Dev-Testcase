/**
 * @file filled_tlb_entry_check.c
 * @author your name (you@domain.com)
 * @brief We intercept the page fault and insert our own TLB entires.
 * 		This testcase is to tset if we successfully inserted the fake PTE into TLB buffer.
 * 		We trigger anonymous page fault on specific virtual range.
 * 		And then, let kernel always insert its previous PTE into the TLB buffer.
 * 		As a result, this app expects getting the previos page's result back instead of its own page.
 * 
 * Expected out results:
 * 
 * ######## User program outputs ########
 * 
 * Request fixed addr 0x400100000000 Reserve user_buffer: 0x400100000000, bytes_len: 0x100000
 * Commit user_buffer: 0x400100000000, bytes_len: 0x100000
 * Phase #1, Initialize the content of each Page.
 * before write: page[0], unsigned long offset[0] : 0
 * Page[0], unsigned long offset[0] : 0
 * 
 * // Full tlb flush
 * before write: page[1], unsigned long offset[0] : 0  // Fill the physical page of page[0], value is 0
 * Prev Page[0], unsigned long offset[0] : 1 // after write on virtual page[1], the original data of physical page[0] is overwriten to 1
 * Page[1], unsigned long offset[0] : 1 // virt page[1] points physical page[0], overwriten to 1
 * 
 * // Full tlb flush
 * before write: page[2], unsigned long offset[0] : 0 // PF, fill physical page[1], empty.
 * Prev Page[1], unsigned long offset[0] : 0 // Write PF on virt page[2], load physical page[2] from primary PT.
 * Page[2], unsigned long offset[0] : 2 // virt page[2] points to physical page[2], overwriten to 2
 * 
 * // Full tlb flush
 * before write: page[3], unsigned long offset[0] : 2 // PF, Fill physical page[2], value 2
 * Prev Page[2], unsigned long offset[0] : 3 // Write hit on physical page[2], overwrite physical page[2] to 3
 * Page[3], unsigned long offset[0] : 3 // virt page[3] points physical [2], overwriten to 3
 * 
 * // Full tlb flush
 * before write: page[4], unsigned long offset[0] : 0 
 * Prev Page[3], unsigned long offset[0] : 0
 * Page[4], unsigned long offset[0] : 4
 * 
 * // Full tlb flush
 * before write: page[5], unsigned long offset[0] : 4
 * Prev Page[4], unsigned long offset[0] : 5
 * Page[5], unsigned long offset[0] : 5
 * 
 * 
 * 
 * 
 * ######## Kernel logs ########
 * 
 * // read fault, push fake TLB w/o dirty bit
 * [Oct20 10:29] arch_push_to_tlb, core #37 fault on 0x400100000000,
 *                                 original pte 0x8000000002e6b205,
 *                                 pushed fake pte addr 0xffff88810f96b000,
 *                                 val 0x8000000002e6b225
 * [  +0.000009] cur_page, pte 0x8000000002e6b225, P 0x1, A 0x20, D 0x0
 *                 mapped kernel virt 0xffff888002e6b000, the first size_t is 0
 *               user_virt_addr 0x400100000000, value 0
 * [  +0.000005] arch_push_to_tlb, core #37 read usr virt 0x400100000000
 * 
 * // write on  the fake pte w/o dirty bit, trigger another page fault
 * [  +0.000054] prev_page, pte 0x800000305cec7847, P 0x1, A 0x0, D 0x40
 *                 mapped kernel virt 0xffff88b05cec7000, the first size_t is 0
 *                 user_virt_addr 0x4000fffff000, value 0
 * [  +0.000004] arch_push_to_tlb, core #37 fault on 0x400100000000,
 *                                 original pte 0x800000305cec7867,
 *                                 pushed fake pte addr 0xffff88810f96b000,
 *                                 val 0x800000305cec7867
 * [  +0.000004] cur_page, pte 0x800000305cec7867, P 0x1, A 0x20, D 0x40
 *                 mapped kernel virt 0xffff88b05cec7000, the first size_t is 0
 *                 user_virt_addr 0x400100000000, value 0
 * [  +0.000004] arch_push_to_tlb, core #37 read usr virt 0x400100000000
 * 
 * 
 * 
 * // read virt page[1], pushed fake pte of virt page[0], w/ dirty bit
 * [  +0.000010] exchange_pte_val_to_previous, exchange the addr 0x400100001000
 *                         to prev_addr 0x400100000000 's pte 0x800000305cec7867
 *                         P 0x1, A 0x20, D 0x40
 * [  +0.000004] prev_page, pte 0x800000305cec7847, P 0x1, A 0x0, D 0x40
 *                 mapped kernel virt 0xffff88b05cec7000, the first size_t is 0
 *                 user_virt_addr 0x400100000000, value 0
 * [  +0.000003] arch_push_to_tlb, core #37 fault on 0x400100001000,
 *                                 original pte 0x8000000002e6b205,
 *                                 pushed fake pte addr 0xffff88810f96b008,
 *                                 val 0x800000305cec7867
 * [  +0.000004] cur_page, pte 0x800000305cec7867, P 0x1, A 0x20, D 0x40
 *                 mapped kernel virt 0xffff88b05cec7000, the first size_t is 0
 *                 user_virt_addr 0x400100001000, value 0
 * [  +0.000003] arch_push_to_tlb, core #37 read usr virt 0x400100001000
 * 
 * 
 * // read virt page[2], pushed fake pte of virt page[1], w/o dirty bit
 * [  +0.000012] exchange_pte_val_to_previous, exchange the addr 0x400100002000
 *                         to prev_addr 0x400100001000 's pte 0x8000000002e6b205
 *                         P 0x1, A 0x0, D 0x0
 * [  +0.000003] arch_push_to_tlb, core #37 fault on 0x400100002000,
 *                                 original pte 0x8000000002e6b205,
 *                                 pushed fake pte addr 0xffff88810f96b010,
 *                                 val 0x8000000002e6b225
 * [  +0.000004] cur_page, pte 0x8000000002e6b225, P 0x1, A 0x20, D 0x0
 *                 mapped kernel virt 0xffff888002e6b000, the first size_t is 0
 *                 user_virt_addr 0x400100002000, value 0
 * [  +0.000003] arch_push_to_tlb, core #37 read usr virt 0x400100002000
 * 
 * 
 * // write virt page[2], fake pte w/o dirty bit, trigger another page fault
 * [  +0.000008] exchange_pte_val_to_previous, exchange the addr 0x400100002000
 *                         to prev_addr 0x400100001000 's pte 0x8000000002e6b205
 *                         P 0x1, A 0x0, D 0x0
 * [  +0.000005] prev_page, pte 0x8000000002e6b205, P 0x1, A 0x0, D 0x0
 *                 mapped kernel virt 0xffff888002e6b000, the first size_t is 0
 *                 user_virt_addr 0x400100001000, value 0
 * [  +0.000002] arch_push_to_tlb, core #37 fault on 0x400100002000,
 *                                 original pte 0x800000305ce8e867,
 *                                 pushed fake pte addr 0xffff88810f96b010,
 *                                 val 0x8000000002e6b225
 * [  +0.000004] cur_page, pte 0x8000000002e6b225, P 0x1, A 0x20, D 0x0
 *                 mapped kernel virt 0xffff888002e6b000, the first size_t is 0
 *                 user_virt_addr 0x400100002000, value 0
 * [  +0.000003] arch_push_to_tlb, core #37 read usr virt 0x400100002000
 * 
 * 
 * // read virt page[3], pushed fake pte of virt page[2], w/ dirty bit
 * [  +0.000012] exchange_pte_val_to_previous, exchange the addr 0x400100003000
 *                         to prev_addr 0x400100002000 's pte 0x800000305ce8e867
 *                         P 0x1, A 0x20, D 0x40
 * [  +0.000004] prev_page, pte 0x800000305ce8e847, P 0x1, A 0x0, D 0x40
 *                 mapped kernel virt 0xffff88b05ce8e000, the first size_t is 2
 *                 user_virt_addr 0x400100002000, value 2
 * [  +0.000003] arch_push_to_tlb, core #37 fault on 0x400100003000,
 *                                 original pte 0x8000000002e6b205,
 *                                 pushed fake pte addr 0xffff88810f96b018,
 *                                 val 0x800000305ce8e867
 * [  +0.000003] cur_page, pte 0x800000305ce8e867, P 0x1, A 0x20, D 0x40
 *                 mapped kernel virt 0xffff88b05ce8e000, the first size_t is 2
 *                 user_virt_addr 0x400100003000, value 2
 * [  +0.000003] arch_push_to_tlb, core #37 read usr virt 0x400100003000
 * 
 * //
 * [  +0.000016] exchange_pte_val_to_previous, exchange the addr 0x400100004000
 *                         to prev_addr 0x400100003000 's pte 0x8000000002e6b205
 *                         P 0x1, A 0x0, D 0x0
 * [  +0.000003] arch_push_to_tlb, core #37 fault on 0x400100004000,
 *                                 original pte 0x8000000002e6b205,
 *                                 pushed fake pte addr 0xffff88810f96b020,
 *                                 val 0x8000000002e6b225
 * [  +0.000003] cur_page, pte 0x8000000002e6b225, P 0x1, A 0x20, D 0x0
 *                 mapped kernel virt 0xffff888002e6b000, the first size_t is 0
 *                 user_virt_addr 0x400100004000, value 0
 * [  +0.000003] arch_push_to_tlb, core #37 read usr virt 0x400100004000
 * [  +0.000008] exchange_pte_val_to_previous, exchange the addr 0x400100004000
 *                         to prev_addr 0x400100003000 's pte 0x8000000002e6b205
 *                         P 0x1, A 0x0, D 0x0
 * [  +0.000005] prev_page, pte 0x8000000002e6b205, P 0x1, A 0x0, D 0x0
 *                 mapped kernel virt 0xffff888002e6b000, the first size_t is 0
 *                 user_virt_addr 0x400100003000, value 0
 * [  +0.000002] arch_push_to_tlb, core #37 fault on 0x400100004000,
 *                                 original pte 0x800000305ce6f867,
 *                                 pushed fake pte addr 0xffff88810f96b020,
 *                                 val 0x8000000002e6b225
 * [  +0.000004] cur_page, pte 0x8000000002e6b225, P 0x1, A 0x20, D 0x0
 *                 mapped kernel virt 0xffff888002e6b000, the first size_t is 0
 *                 user_virt_addr 0x400100004000, value 0
 * [  +0.000002] arch_push_to_tlb, core #37 read usr virt 0x400100004000
 * 
 * 
 * 
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
		printf("before write: read on page[%lu], unsigned long offset[%d] : %lu\n",
			j/(PAGE_SIZE/sizeof(unsigned long)), 0,	buf_ptr[j+0]);

		for(i= 0; i<(PAGE_SIZE/sizeof(unsigned long)) ; i++ ){
			// Initialize the content of the page to the page num.
			// Trigger a page fault here.
			// The kernel change the pte value to the its previous physical page.
			// Skip the first page.
			buf_ptr[j+i] = j/(PAGE_SIZE/sizeof(unsigned long));  
		}

		// ...
		if(j >= PAGE_SIZE/sizeof(unsigned long)){
			// previous page
			// We overwrite the data on previous page
			printf("After write, Read on Prev Page[%lu], unsigned long offset[%d] : %lu \n",
				j/(PAGE_SIZE/sizeof(unsigned long)) -1 , 0, buf_ptr[j - (PAGE_SIZE/sizeof(unsigned long)) +0]);
		}
		
		// current page
		printf("After write, Read on Page[%lu], unsigned long offset[%d] : %lu \n\n",
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
