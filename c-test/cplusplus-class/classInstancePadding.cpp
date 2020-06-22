/**
 * Check the address of each field.
 * Understand the C++ instance padding policy.	
 * 
 */

// mmap
#include <sys/mman.h>

#include "stdio.h"
// malloc
#include "stdlib.h"

class padding_class{
public:

	int integer_1;		// 4 bytes, padding 4 bytes.
	double double_1;	// 8 bytes,

	padding_class(){
		integer_1 = -1;
		double_1 = -1.0;
	}

};


class no_padding_class{
public:

	int integer_1;	// 4 bytes,
	int integer_2;	// 4 bytes,

	no_padding_class(){
		integer_1 = -1;
		integer_2 = -2;
	}

};



int main(int argc, char* argv[]){

	padding_class obj_padding;			// Allocate instance into heap.
	no_padding_class obj_no_padding;

	printf(" padding_class object : address 0x%lx , size 0x%lx. filed#integer_1 0x%lx, field#double_1 0x%lx \n", 
									(size_t)&obj_padding, (size_t)sizeof(obj_padding),
									(size_t)&(obj_padding.integer_1), (size_t)&(obj_padding.double_1) );
	printf(" no_padding_class object address :0x%lx	, size 0x%lx. filed#integer_1 0x%lx, field#integer_2 0x%lx \n", 
									(size_t)&obj_no_padding, (size_t)sizeof(obj_no_padding), 
									(size_t)&(obj_no_padding.integer_1), (size_t)&(obj_no_padding.integer_2)  );

	printf("\n");

	return 0;
}
