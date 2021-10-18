#include "stdio.h"
#include "stdlib.h"

int main(int argc, char* argv[]){
  int *pointer; 
  
  while(1){
    pointer = (int*)malloc(sizeof(int));
    *pointer = 1;
    int read_pointer = *pointer;
    printf("value of pointer is : %d \n", read_pointer);
    
    // Failed to release data cause memory leak !
  }
  return 0;
}



