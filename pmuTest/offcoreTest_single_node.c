
/*
* Test the platform's offcore event
* 1) NUMA local / remote memory access test
* 2) All local/ All remote 
* 
* Warning:
* 1) Compile with -O0
*
*/


#include "stdio.h"


#define ARRAY_SIZE	1024*1024*64



int main(int argc, char* argv[]){

  double* arr_1 = (double*)malloc(sizeof(double)* ARRAY_SIZE);
  double sum = 0; 
  int i,j,k;

  // initiate the array
  // The initialization write may not cause any RFO LLC Miss in new architecture
  // It will all hit the LLC, works like Write Though
  // write
  for(i=0; i<ARRAY_SIZE; i++){
	arr_1[i] = i;
  }

  for(j=0; j<16;j++){
  // read 
  // ARRAY_SIZE*8/64 times DEMAND_READ LLC miss response
	for(i=0; i<ARRAY_SIZE; i+=8 ){  // 64 bytes cache line
	  sum += arr_1[i]*i;
	}

	// write again
	// ARRAY_SIZE*8/64 times DEMAND_RFO LLC Miss response
	for(i=0; i<ARRAY_SIZE; i++){
	  arr_1[i] = i*i;
	}

  }

  // prevent opmizaing the calculation
  printf("Sum is %f \n",sum);
	 
  return 0;
}


