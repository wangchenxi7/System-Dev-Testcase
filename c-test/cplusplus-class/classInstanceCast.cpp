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

#include "string.h"
#include "stdio.h"
// malloc
#include "stdlib.h"

class inner_class{
public:

	int integer_1;		// 4 bytes, padding 4 bytes.
	double double_1;	// 8 bytes,

	inner_class(){
		integer_1 = -1;
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

	outside_class* obj_outside_pointer = (outside_class*)malloc(sizeof(outside_class)); // malloc doesn't inoke the Constructor
	outside_class obj_outside_var;



	printf("obj_outside_pointer : allocated by malloc : \n");
	printf(" obj_outside_pointer : address 0x%lx , size 0x%lx \n", (unsigned long)obj_outside_pointer, (unsigned long)sizeof(obj_outside_pointer) );
	printf("		obj_outside_pointer->obj_inner_var :0x%lx	, size 0x%lx \n", (unsigned long)&(obj_outside_pointer->obj_inner_var), (unsigned long)sizeof(obj_outside_pointer->obj_inner_var) );
	printf("				obj_outside_pointer->obj_inner_var.integer_1 %d \n",obj_outside_pointer->obj_inner_var.integer_1);
	printf("				obj_outside_pointer->obj_inner_var.double_1 %f \n",obj_outside_pointer->obj_inner_var.double_1);
	printf("		obj_outside_pointer->obj_inner_pointer : 0x%lx, size 0x%lx \n	",(unsigned long)obj_outside_pointer->obj_inner_pointer, (unsigned long)sizeof(obj_outside_pointer->obj_inner_pointer) );
	
	if(obj_outside_pointer->obj_inner_pointer != 0x0){
		printf("				obj_outside_pointer->obj_inner_pointer->integer_1 %d \n", obj_outside_pointer->obj_inner_pointer->integer_1);
		printf("				obj_outside_pointer->obj_inner_pointer->double_1 %f \n", obj_outside_pointer->obj_inner_pointer->double_1);
	}else{
		printf("				Can't print a pointer with 0x0. \n");
	}
	printf("\n");


	// Copy obj_outside_var to obj_outside_pointer.
	memcpy(obj_outside_pointer, &obj_outside_var, sizeof(outside_class));

	printf("obj_outside_pointer : after memcpy : \n");
	printf(" obj_outside_pointer : address 0x%lx , size 0x%lx \n", (unsigned long)obj_outside_pointer, (unsigned long)sizeof(obj_outside_pointer) );
	printf("		obj_outside_pointer->obj_inner_var :0x%lx	, size 0x%lx \n", (unsigned long)&(obj_outside_pointer->obj_inner_var), (unsigned long)sizeof(obj_outside_pointer->obj_inner_var) );
	printf("				obj_outside_pointer->obj_inner_var.integer_1 %d \n",obj_outside_pointer->obj_inner_var.integer_1);
	printf("				obj_outside_pointer->obj_inner_var.double_1 %f \n",obj_outside_pointer->obj_inner_var.double_1);
	printf("		obj_outside_pointer->obj_inner_pointer : 0x%lx, size 0x%lx \n	",(unsigned long)obj_outside_pointer->obj_inner_pointer, (unsigned long)sizeof(obj_outside_pointer->obj_inner_pointer) );
	printf("				obj_outside_pointer->obj_inner_pointer->integer_1 %d \n", obj_outside_pointer->obj_inner_pointer->integer_1);
	printf("				obj_outside_pointer->obj_inner_pointer->double_1 %f \n", obj_outside_pointer->obj_inner_pointer->double_1);
	printf("\n");
	

	printf("The compared value:");
	printf(" obj_outside_var : address 0x%lx , size 0x%lx \n", (unsigned long)&obj_outside_var, (unsigned long)sizeof(obj_outside_var) );
	printf("		obj_outside_var.obj_inner_var :0x%lx	, size 0x%lx \n", (unsigned long)&(obj_outside_var.obj_inner_var), (unsigned long)sizeof(obj_outside_var.obj_inner_var) );
	printf("				obj_outside_var.obj_inner_var.integer_1 %d \n",obj_outside_var.obj_inner_var.integer_1);
	printf("				obj_outside_var.obj_inner_var.double_1 %f \n",obj_outside_var.obj_inner_var.double_1);
	printf("		obj_outside_var.obj_inner_pointer : 0x%lx, size 0x%lx \n	",(unsigned long)obj_outside_var.obj_inner_pointer, (unsigned long)sizeof(obj_outside_var.obj_inner_pointer) );
	printf("				obj_outside_var.obj_inner_pointer->integer_1 %d \n", obj_outside_var.obj_inner_pointer->integer_1);
	printf("				obj_outside_var.obj_inner_pointer->double_1 %f \n", obj_outside_var.obj_inner_pointer->double_1);
	


	return 0;
}
