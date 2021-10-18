#include "stdio.h"
#include "stdlib.h"

int main(int argc, char* argv[]){
  int *pointer = (int*)malloc(sizeof(int));
  *pointer = 1;

  // Releasing data prematurely causes dangling reference (pointer)
  free(pointer);

  int read_pointer = *pointer;
  printf("value of pointer is : %d \n", read_pointer);

  return 0;
}

