#include "stdio.h"

int main(int argc, char* argv[]){

  int a = 8;
  int & b = a;
  int * c = &a;

  printf(" addr of a 0x%lx, addr of b 0x%lx, value of c 0x%lx \n", (size_t)(&a), (size_t)(&b), (size_t)c );

  return 0;

}