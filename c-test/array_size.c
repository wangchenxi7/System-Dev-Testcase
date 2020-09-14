/**
 * [?] How does the (long unsigned int) sizeof() know the size of an array by passing the pointer ?
 *  a. local/global array can be detected correctly by sizeof.
 *  b. for pointer pointing to memory allocated by malloc, sizeof returns the size of the pointer. 
 */


#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main(int argc, char* argv[]){

	int a[3];
	int *b = (int*)malloc(sizeof(int) * 5);
	int i;

	printf("size of array int a[3]:%lu bytes \n", sizeof(a));
	printf("size of array int *b = malloc(sizeof(int)*5):%lu bytes \n", sizeof(b));


	// memset
	//memset(&a, -1, sizeof(a));
	memset(a, -1, sizeof(a));    // For a single dimension array, a and &a points to the same address. 
	memset(b,-1,sizeof(int)*5 );

	printf("&a : 0x%lx \n",(size_t)&a);
	printf("a : 0x%lx \n",(size_t)a);   //same, both are address on stack
	printf("&b : 0x%lx \n",(size_t)&b);	
	printf("b : 0x%lx \n",(size_t)b);		// different.  b points to the first element of array, on heap. &p is the variable on stack

	for(i=0; i<3; i++)
		printf("a[%d] :%d \n",i, a[i]);

	for(i=0; i<5; i++)
		printf("b[%d]: %d \n",i,b[i]);

	return 0;
}
