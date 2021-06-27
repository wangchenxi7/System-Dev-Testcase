/**
 * Get the byte offset of a field within a struct.
 * The thoughts here is to cast address 0 to a local/temporay struct
 * and then print the address of its field.
 * So, the printed virtual address is the offset of each field.
 * 
 * Question
 * 	1) (TYPE *)0 is not a null pointer ?
 * 		&( pointer->field) is not derefering the field. it only gets the virtual address of the field.
 * 	2) -> has higher priority than &
 * 
 * 
 */
#include "stdio.h"

struct test_struct
{
	int priority; 

	struct test_struct * prev;
	struct test_struct * next;
};



#define offsetof(TYPE, MEMBER)	((size_t)&((TYPE *)0)->MEMBER)



int main(int argc, char* argv){

	printf(" virtual address of (TYPE*)0 : 0x%lx \n", (size_t)(struct test_struct*)0 );
	
	// get the virtual address of each field.
	// here we don't dereference the field, only get its address.
	printf(" offset of field priority 0x%lx bytes\n", offsetof(struct test_struct, priority)  );
	printf(" offset of field prev 0x%lx bytes\n", offsetof(struct test_struct, prev)  );
	printf(" offset of field next 0x%lx bytes\n", offsetof(struct test_struct, next)  );

	// The access of the field will cause segmentation fault
	printf("\n Going to access the field of the struct and trigger a SEGMETNV \n");
	printf(" virtual address of (TYPE*)0 : 0x%lx \n", (size_t)((struct test_struct*)0)->priority );


	return 0;
}



