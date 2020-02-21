#include "stdio.h"

int max(int a, int b){
	return a >= b ? a: b; 
}


int main(int argc, char* argv[]){

	int (*func_p)(int, int) = &max;

	printf(" max(5,7) = %d \n", func_p(5,7) );


	return 0;
}