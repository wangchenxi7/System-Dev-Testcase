#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>  
#include <sys/errno.h>

#include "stdint.h"
#include "stdio.h"



#define SYS_SWAP_STAT_READ		452


int main(){
				
	int ondemand_swapin_num = 0;
	int prefetch_swapin_num = 0;
	int hit_on_prefetch_num = 0;


	printf("	#1 Check current swapped out pages num\n");
	unsigned long on_demand_swapin = syscall(SYS_SWAP_STAT_READ, &ondemand_swapin_num, &prefetch_swapin_num, &hit_on_prefetch_num);
	printf("	#1 on-demand swapin pages %d , prefetch swapin pages %d, hit-on swap cache pages %d \n", 
							ondemand_swapin_num, prefetch_swapin_num, hit_on_prefetch_num);  // should be 0.

  return 0;

}