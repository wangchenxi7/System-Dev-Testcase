/**
 * Check the address for class instance under different allocation way.
 *  e.g. 
 *   class foo{}
 *   foo obj_foo#1 = new foo();
 *   foo obj_foo#2;		
 * 
 */

// mmap
#include <sys/mman.h>

#include "stdio.h"
// malloc
#include "stdlib.h"

class inner_class{
public:

	int interger_1;		// 4 bytes, padding 4 bytes.
	double double_1;	// 8 bytes,

	inner_class(){
		interger_1 = -1;
		double_1 = -1.0;
	}

};

class outside_class{
public:

	double double_2;									// 8 bytes,

	inner_class obj_inner_var;				// 16 bytes, its instance address should be outside_class's instance + fixed_offset.
	inner_class *obj_inner_pointer;   // 8 bytes, its instance's address have nothing to do outside_class.


	outside_class(){
		// new the inner_class instance
		obj_inner_pointer = new inner_class();
	}

};


int main(int argc, char* argv[]){

	outside_class* obj_outside_pointer = new outside_class();			// Allocate instance into heap.
	outside_class obj_outside_var;						// allocate the instance into stack.

	printf(" obj_outside_pointer : address 0x%lx , size 0x%lx \n", 
									(unsigned long)obj_outside_pointer, (unsigned long)sizeof(obj_outside_pointer) );
	printf("		obj_outside_pointer->obj_inner_var :0x%lx	, size 0x%lx \n", 
									(unsigned long)&(obj_outside_pointer->obj_inner_var), (unsigned long)sizeof(obj_outside_pointer->obj_inner_var) );
	printf("		obj_outside_pointer->obj_inner_pointer : 0x%lx, size 0x%lx \n	",
									(unsigned long)obj_outside_pointer->obj_inner_pointer, (unsigned long)sizeof(obj_outside_pointer->obj_inner_pointer) );
	printf("\n");


	printf(" obj_outside_var : address 0x%lx , size 0x%lx \n", (unsigned long)&obj_outside_var, (unsigned long)sizeof(obj_outside_var) );
	printf("		obj_outside_var.obj_inner_var :0x%lx	, size 0x%lx \n", (unsigned long)&(obj_outside_var.obj_inner_var), (unsigned long)sizeof(obj_outside_var.obj_inner_var) );
	printf("		obj_outside_var.obj_inner_pointer : 0x%lx, size 0x%lx \n	",(unsigned long)obj_outside_var.obj_inner_pointer, (unsigned long)sizeof(obj_outside_var.obj_inner_pointer) );
	


	return 0;
}
