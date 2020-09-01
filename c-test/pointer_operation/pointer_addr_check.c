#include "stdio.h"


int main(int argc, char* argv[] ){

	char *addr =(char*)0xdeafbabedeafbabe;
	size_t comp =0xd;

	if((size_t)addr >> 60 == comp ){
		printf("Equal.\n");
	}else{
		printf("Not Equal. \n");
	}
	
	printf(" addr: 0x%lx.  (size_t)addr >> 60 0x%lx \n",(size_t)addr, (size_t)addr >>60 );
	printf(" comp : 0x%lx \n",comp);
	
	return 0;

}
