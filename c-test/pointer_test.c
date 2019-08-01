#include "stdio.h"

// 
struct inner_data{
  int int_a;
  double double_a;
  char char_a;
};


struct request{
  struct inner_data filed_1;
  union{
    long long_field;
    unsigned int int_field;
  }; 
};



int main(int argc, char* argv[]){

  struct request* tmp;


  tmp = (struct request*)malloc(sizeof(struct request));

  tmp->filed_1.int_a = 1;
  tmp->filed_1.double_a = 2.0;
  tmp->filed_1.char_a = "3";

  tmp->int_field = 4;


  printf("dump the struct request tmp: \n");
  printf("tmp->filed_1.int_a : %d \n", tmp->filed_1.int_a);
  printf("tmp->filed_1.double_a : %f \n ", tmp->filed_1.double_a);
  printf("tmp->filed_1.char_a : %c \n", tmp->filed_1.char_a);
  printf("tmp->int64_field : %u \n",tmp->int_field);

  printf("size of struct request : %d bytes\n", sizeof(struct request));
  printf(" tmp : %llx \n", (void*)tmp);             // base
  printf(" tmp +1 : %llx \n ", (void*)(tmp+1) );    // base +0x20 ,  32  bytes, sizeof(struct request)


  return 0;
}