#include "stdio.h"
#include "stdlib.h"

#define ARRAY_LENGTH 1024*1024*1024*1

int main(int argc, char* argv[]){

	int i,j,k;
	double* a = (double*)malloc(sizeof(double) * ARRAY_LENGTH);
	double* b = (double*)malloc(sizeof(double) * ARRAY_LENGTH);
	double sum;
		
	j=0;	
	while(j<5){

	for(i=0; i< ARRAY_LENGTH; i++){
		a[i]=i*3.14*1;
		b[i]=i*3.14*2;
	}

	for(i=0; i< ARRAY_LENGTH; i+=8){
		sum += a[i];
		sum += b[i];
	}

	for(i=0; i<ARRAY_LENGTH; i+=1024*1024*64){
		printf("a[%d] : %f \n",i,a[i]);
		printf("b[%d] : %f \n",i,b[i]);
	}

		j++;
	}

	return 0;
}
