/**
* Testcase for kernel development
*	1) Trigger page fault - anoymous  
*	2) Try to trigger the paging for write & reead
*		=> Need to limit the available memory for QEMU
*/

#include "stdio.h"
#include "stdlib.h"


#define  ARRAY_LENGTH 256*1024*1024


int main(int argc, char* argv[]){

	double* a = (double*)malloc(ARRAY_LENGTH*sizeof(double));

	int i,j,k;
	double sum = 0;

	printf("Stage 1: Trigger anoymous page fault \n");
	// 1) Trigger page fualt - anoymous
	// 2) Trigger the paging out
	for(i=0;i<ARRAY_LENGTH;i+=8){
		a[i]= i*3.14;
		sum += a[i];
	}

	printf("State 2 : try to access the paged out pages. \n");

	// 3) Triger the paging in
	// Prevent optimization
	for(j=0;j<ARRAY_LENGTH;j+=128*1024){
		printf("a[%d] : %f \n",j,a[j]);
	}

	printf("Sum is : %d \n", sum);

	return 0;
}
