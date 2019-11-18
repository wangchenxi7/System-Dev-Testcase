#include "stdio.h"

/**
 * variable/flexible array in struct
 * https://stackoverflow.com/questions/2060974/how-to-include-a-dynamic-array-inside-a-struct-in-c
 * 
 */
struct mydata {
    // various other data fields
    int varDataSize;
    char data[];				// A flexible array. points to the memory space next it.
};


int main(int argc, char* argv[]){

	struct mydata * tmp = malloc(sizeof(struct mydata) + 64);

	printf("Size of the struct mydata : %d \n", sizeof(struct mydata));   // 4 bytes, only contain  the int ?
	//printf("Size of the mydata->data[] :  %d \n", sizeof(tmp->data));		// Can't get. But it points the next 64 bytes.

	printf("\n\n");
	printf("Start address of entire struct : 0x%llx \n", tmp);													// offset 0x0
	printf("Start address of the mydata->varDataSize : 0x%llx \n", &tmp->varDataSize);	// offset 0x0			
	printf("Start address of the mydata->data : 0x%llx \n", tmp->data);									// offset +0x4


	return 0;
}
