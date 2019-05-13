/**
* Testcase for kernel development
*	1) Trigger page fault - anoymous  
*	2) Try to trigger the paging for write & reead
*		=> Need to limit the available memory for QEMU
*/

#include "stdio.h"
#include "stdlib.h"
//kernel header
#include "fcntl.h"
#include "errno.h"
//#include "asm/uaccess.h"
//#include "linux/buffer_head.h"



#define  ARRAY_LENGTH 512*1024*1024  // length of array,  8 bytes/item


int main(int argc, char* argv[]){

	double* a = (double*)malloc(ARRAY_LENGTH*sizeof(double));

	int i,j,k;
	double sum = 0;

	// Stage 1 : Swap out page
	printf("Stage 1: Trigger anoymous page fault \n");
	// 1) Trigger page fualt - anoymous
	// 2) Trigger the paging out
	for(i=0;i<ARRAY_LENGTH;i+=8){
		a[i]= i*3.14;
		sum += a[i];
	}

	
	// Stage 2 : Swap in page
	printf("State 2 : try to access the paged out pages. \n");

	// [?] Trigger a special path to let GDB stop here ?
	int fd = open("foo.txt", O_RDONLY | O_CREAT); 
	printf("Syscall at start of state 2, open- fd : %d \n", fd);

	// 3) Triger the paging in
	// Prevent optimization
	for(j=0;j<ARRAY_LENGTH;j+=1024*1024){
		printf("a[%d] : %f \n",j,a[j]);
	}

	printf("Sum is : %d \n", sum);

	return 0;
}
