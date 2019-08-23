#include "stdio.h"


#define CHUNK_SIZE_GB_1 1       // 0 bit
#define CHUNK_SIZE_GB_2 2       // 1 bit
#define CHUNK_SIZE_GB_4 4       // 2 bits 

// C version
unsigned long long power_of_2(unsigned long long  num){
    
    unsigned long long  power = 0;

    while( (num = (num >> 1)) !=0 ){
        power++;
    }

    return power;
}


// Corresponding Macro version



int main(int argc, char* argv[]){

    //uint64_t gb_offset;

    printf("offset for CHUNK_SIZE_GB_1 : %d \n", 30 + power_of_2(CHUNK_SIZE_GB_1) );

    printf("offset for CHUNK_SIZE_GB_2 : %d \n", 30 + power_of_2(CHUNK_SIZE_GB_2) );

    printf("offset for CHUNK_SIZE_GB_4 : %d \n", 30 + power_of_2(CHUNK_SIZE_GB_4) );

    return 0;
}