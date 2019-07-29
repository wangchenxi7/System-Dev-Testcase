#include "stdio.h"


/**
 *  divide n by b
 * 
 * 1) change the value of "n" : n/=b; 
 * 2) return value is n%b;
 * 
 */
# define sector_div(n, b)( \
{ \
	int _res; \
	_res = (n) % (b); \
	(n) /= (b); \
	_res; \
} \
)


int main(int argc, char* argv[]){

  int n = 1023, b =128;
  int ret = 0;

  ret = n%b;
  printf("n\%b is : %d \n", ret );

  ret = n/b;
  printf("n/b is : %d \n", ret );


  ret = sector_div(n,b);
  printf("sector_div(n,b) return value : %d, n is : %d ,b is %d \n",ret, n,b);



  return 0;
}



