#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main(int argc, char* argv[] ){

	char* 	mem_server_ip[] = {"10.0.0.2", "10.0.0.4"};
	int i;

	for(i=0;i<2;i++){
		printf("mem_server_ip[%d] %s, len %lu \n",i, mem_server_ip[i], strlen(mem_server_ip[i]) );
	}


	return 0;
}
