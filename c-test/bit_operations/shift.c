#include "stdio.h"


#define RMEM_LOGICAL_SECT_SHIFT 0x9

int main(int argc, char* argv[]){

    unsigned long long a = 0xfffff8;


    printf("a:0x%llx, RMEM_LOGICAL_SECT_SHIFT: 0x%llx, a<<RMEM_LOGICAL_SECT_SHIFT:0x%llx \n ",
                                        a, RMEM_LOGICAL_SECT_SHIFT, a<<RMEM_LOGICAL_SECT_SHIFT);


    printf("a<<8:0x%llx, a<<9:0x%llx \n",a<<8, a<<9);

    return 0;
}

