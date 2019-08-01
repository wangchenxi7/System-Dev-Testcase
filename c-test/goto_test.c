/**
 * Test the execution sequence. 
 * 
 * not work like if - else if - else,
 * if not jump to the later flag, 
 * all the code under flags will be executed. 
 * 
 */

#include "stdio.h"



int main(int argc, char* argv[]){

  int a =1;

  printf(" a is with flag_%d \n",a);
  if ( a == 1){
    goto flag_1;
  }else if(a == 5){
    goto flag_5;
  }

  // !!Warning!!
  // if not jump to flag_5,
  // the code under flag_1 will be executed.
  flag_1 : 
    printf("under flag_1 \n");


  flag_5 :
    printf("under flag_5 \n");

  out:
    printf("Finished. \n");

  return 0;
}