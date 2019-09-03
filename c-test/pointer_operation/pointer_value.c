#include "stdio.h"


#define ARRAY_LENGTH 10

struct page{
    int a;
    double b;
    char c;
};


int main(int argc, char* argv[]){

    struct page * struct_array;
    struct page * ptr;
    int i,j;

    struct_array = (struct page*)malloc( sizeof(struct page) * ARRAY_LENGTH );

    for(i=0; i<ARRAY_LENGTH; i++){

       
        struct_array[i].a = i;
        struct_array[i].b = i;
        struct_array[i].c = "i";

        printf(" struct_array[%d] : 0x%llx \n", i, struct_array[i]);    // the value of firt 64 bits of address &struct_array[i]
        printf(" &struct_array[%d] : 0x%llx \n", i, &struct_array[i]);

        if(i  == 3){
            ptr = &struct_array[i];
            printf("struct_array + %d :0x%llx \n",i, struct_array+i);
        }
    }

    printf("ptr (points to &struct_array[i]):0x%llx \n",ptr);




    return 0;
}