
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>  
#include <sys/errno.h>
#include </usr/include/asm-generic/mman-common.h>

#include "stdint.h"
#include "stdio.h"



#define SYS_SWAP_STAT_RESET			335
#define SYS_ON_DEMAND_SWAPIN		337



int main(){
				
	int type = 0x1;
	uint64_t request_addr 	= 0x40000000; // start of RDMA meta space, 1GB not exceed the swap partitio size.
	uint64_t size  					=	0x40000000;		// 1GB data


  printf("	Reset the swap out statistics monitoring array\n");
	int ret = syscall(SYS_SWAP_STAT_RESET, request_addr, size);
	printf("	SYS_SWAP_STAT_RESET returned %d \n", ret);

	printf("	#1 Check current swapped out pages num\n");
	size_t on_demand_swapin = syscall(SYS_ON_DEMAND_SWAPIN);
	printf("	#1 on-demand swap-in pages num 0x%llx \n", on_demand_swapin);  // should be 0.




  return 0;

}