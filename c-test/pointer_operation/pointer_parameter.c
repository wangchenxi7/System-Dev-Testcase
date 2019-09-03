/**
 * Test the usage for pointer paramter
 * 
 * 1) When using a pointer as parameter, it just copy its valut as the paramter.
 * i.e.
 * for  init_internal_pointer_2(p_2), it only get the NULL (value ) of p_2.
 * And the malloc can't assign the allocated memory to p_2.
 * 
 * 2) the value of variable isn't initialized sometimes.
 * 
 */

#include "stdio.h"

struct internal{
  int a;
  double b;
};


void init_internal_pointer_1(struct internal **p_ptr){
  if (*p_ptr == NULL){
    *p_ptr = (struct internal*)malloc(sizeof(struct internal));
    ((struct internal*)(*p_ptr))->a = 7;
    ((struct internal*)(*p_ptr))->b = 7.7;
  }

  return;
}

void init_internal_pointer_2(struct internal * p ){

  if (!p){
    p = (struct internal*)malloc(sizeof(struct internal));
    p->a = 7;
    p->b = 7.7;
  }

  return;
}


int main(int argc, char* argv[]){

  struct internal * p_1 = NULL;   // Warning : if not initialize to NULL, it can be a random non-null value.
  struct internal * p_2 = NULL;

  init_internal_pointer_1(&p_1);  // Pass the address of p_1
  init_internal_pointer_2(p_2);   // Pass the value of p_2

  // check if the fields have been initiated. 
  printf("p_1->a : %d \n", p_1->a);   // Segment fault.!!
  printf("p_1->b : %f \n", p_1->b);


  printf("Accessing the fields of p_2 causes segment fault.\n");
  printf("p_2 : 0x%x \n",p_2);
  printf("p_2->a : %d \n", p_2->a);   // Segment fault.!!
  printf("p_2->b : %f \n", p_2->b);


  return 0;
}
