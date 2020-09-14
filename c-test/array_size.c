/**
 * [?] How does the (long unsigned int) sizeof() know the size of an array by passing the pointer ?
 *  a. local/global array can be detected correctly by sizeof.
 *  b. for pointer pointing to memory allocated by malloc, sizeof returns the size of the pointer. 
 */


#include "stdio.h"
#include "stdlib.h"

int main(int argc, char* argv[]){

	int a[3];
	int *b = (int*)malloc(sizeof(int) * 5);

	printf("size of array int a[3]:%lu bytes \n", sizeof(a));
	printf("size of array int *b = malloc(sizeof(int)*5):%lu bytes \n", sizeof(b));

	return 0;
}
