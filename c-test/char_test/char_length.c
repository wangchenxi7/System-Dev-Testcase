/**
 * Test the length of a char* and char[]
 * 
 * the end of a char string has to be NULL, 0x0. 
 * 
 */


#include "stdio.h"
#include "stdlib.h"
#include "string.h"


int main (int argc, char* argv[]){

  char a[5]  = {'a','b','c','d',0x0};
  printf("The length of char a[5] is : %lu \n", sizeof(a) );       // Return the size of array char a[5], it's 5.
  printf("Content %s \n",  a);

  char* b = (char*)malloc(sizeof(char) * 5);
  b = "abcd";
  printf("The length of char* b = (char*)malloc(sizeof(char) * 5) is : %lu \n", sizeof(b));    // Return the size of pointer char* b, it's 8 byts. 
  printf("strlen of b is : %lu \n", strlen(b));
  printf("Content %s \n",  b);

  return 0;
}