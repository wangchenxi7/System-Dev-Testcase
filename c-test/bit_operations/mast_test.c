#include "stdio.h"


int main(int argc, char* argv[]){

    #define GB 30
    #define GB_MASK (1<<GB)-1

    printf("GB_MASK 0x%llx \n",GB_MASK);

    return 0;
}