/**
 *  Get the kernel version information for writing a more compatible code. 
 * 
 * 
 */

#include "stdio.h"

#include <linux/kernel.h>
#include <linux/version.h>


int main(int argc, char* argv[]){

	unsigned cur_kernel_code = LINUX_VERSION_CODE; // 24 bits
	unsigned low_8_bits = ((1<<8) -1);

	printf("the value of KERNEL_VERSION(4,5,0) is %d \n", KERNEL_VERSION(4,5,0));
	printf("Current kernel version code is %u , %u, %u, %u\n", 
		cur_kernel_code, 
		(cur_kernel_code >> 16)&low_8_bits, 
		(cur_kernel_code >> 8)&low_8_bits,
		cur_kernel_code & low_8_bits );



	return 0;
}