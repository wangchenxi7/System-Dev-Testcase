#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>  
#include <sys/errno.h>

#include "stdint.h"
#include "stdio.h"


#define SYS_SWAP_STAT_RESET 451
#define SYS_GET_SWAP_STATS 452


int main()
{
	int ret = 0;

	printf("	Reset the swap out latency statistics\n");
	ret = syscall(SYS_SWAP_STAT_RESET);
	printf("	SYS_SWAP_STAT_RESET returned %d \n", ret);

	printf("	#1 Check current swapped out pages num\n");
	ret = syscall(SYS_GET_SWAP_STATS);
	printf("	SYS_GET_SWAP_STATS returned %d \n", ret); // should be 0.

	return 0;
}