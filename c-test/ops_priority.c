/**
 * Test the priorities of operations.
 */
#include "stdio.h"

int main(int argc, char* argv[]){

    int a = 1;
    int b = 2;
    int c = 3;

    a = b =c;

    printf("a : %d, b:%d, c:%d \n",a, b, c);

    return 0;
}