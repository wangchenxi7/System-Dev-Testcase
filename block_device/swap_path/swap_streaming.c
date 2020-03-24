/**
 * Write a streaming program to trigger the swap out/in path.
 * 
 * 
 */

#include "stdio.h"


#define ARRAY_LENGTH 1024*1024*16   // length is 16M, 

int main(int argc, char* argv[]){
	int i;
	double sum;
	double *d_array = (double *)malloc( sizeof(double) * ARRAY_LENGTH);

	printf("Phase #1, trigger swap out. \n");
	for(i =0; i< ARRAY_LENGTH; i++){
		d_array[i] = i * 3.14;
	}


	printf("Phase #2, trigger swap in.\n");
	sum = 0;
	for(i=0; i < ARRAY_LENGTH; i++){
		sum += d_array[i];
	}


	printf("Sum : %f \n", sum);

	return 0;
}