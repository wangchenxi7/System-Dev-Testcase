/**
 * Use madvice() MAD_FREE to put a specific range of memory into inactive list. 
 * These pages will be swapped out when there is a memory pressure.
 * 
 */

#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>  
#include <sys/errno.h>
#include </usr/include/asm-generic/mman-common.h>
#include <unistd.h>

#include "stdint.h"
#include "stdio.h"


// Semeru
//#include <linux/swap_global_struct.h>

#define MADV_FLUSH_RANGE_TO_REMOTE 20  // Should include from linux header

#define SYS_SWAP_STAT_RESET			335
//#define SYS_NUM_SWAP_OUT_PAGES	336
#define SYS_ON_DEMAND_SWAPIN		337


unsigned int microseconds;

typedef enum {true, false} bool;

extern errno;





int main(){
				
	int type = 0x1;
	unsigned long request_addr 	= 0x40000000; // start of RDMA meta space, 1GB not exceed the swap partitio size.
	unsigned long size  					=	0x100000;		// 1MB, for unsigned long, length is 0x20,000
	//unsigned long size  					=	0x2000000;		// 16MB, for unsigned long, length is 0x200,000
	char* user_buff;
	unsigned long i;
	unsigned long sum = 0;
	unsigned long swapped_out_pages = 0;
	int on_demand_swapin_record = 0;
	int ret;

	// 1) reserve space by mmap
	microseconds=1000000; //1 seconds



	while(1){
		on_demand_swapin_record	= syscall(SYS_ON_DEMAND_SWAPIN);
		printf("on demand swapin page 0x%x\n", on_demand_swapin_record);  // read the on-demand swapin counter
		
		//printf("	Reset the swap out statistics monitoring array\n");
		ret = syscall(SYS_SWAP_STAT_RESET, request_addr, size);  // reset to 0.
		if(ret){
			printf(" ERROR	SYS_SWAP_STAT_RESET returned %d \n", ret);
		}

		usleep(microseconds);
	}

	return 0;
}
