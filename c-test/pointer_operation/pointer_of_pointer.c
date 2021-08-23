/**
 * @file pointer_of_pointer.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-22
 * 
 * @copyright Copyright (c) 2021
 * 
 * 
 * 
 * 
 * 
 */


#include "stdio.h"
#include "stdlib.h"

int main(void)
{

		int **a;
    int *b;

    printf(" &(b) is 0x%lx, a is 0x%lx \n", (size_t)&b, (size_t)a);
    printf("Warning : a can be any random value or null\n");

		a = (int **)malloc(sizeof(int *) * 2);

		a[0] = (int *)malloc(sizeof(int));
		a[1] = (int *)malloc(sizeof(int));

		*a[0] = 10;
		*a[1] = 20;

		printf(" a is 0x%lx , *a is 0x%lx \n", (size_t)a, (size_t)(*a));
		printf(" a + 1 is 0x%lx, *(a+1) is 0x%lx \n", (size_t)(a + 1),
		       (size_t) * (a + 1));
		printf(" *(a[0]) is %d, a[0] is 0x%lx, &a[0] is 0x%lx \n", *(a[0]),
		       (size_t)a[0], (size_t)&a[0]);
		printf(" *(a[1]) is %d, a[1] is 0x%lx, &a[1] is 0x%lx \n", *(a[1]),
		       (size_t)a[1], (size_t)&a[1]);
		printf(" *a[1] is %d, &(*a)[1] is 0x%lx \n", *a[1], (size_t) & (*a)[1]);

		printf(" *a[1] is %d, &(*a) + 1 is 0x%lx \n", *a[1],
		       (size_t)(&(*a) + 1));
		printf(" *a[1] is %d, (int**)&(*a) + 1 is 0x%lx \n", *a[1],
		       (size_t)(((int **)&(*a)) + 1));

		return 0;
}

/**
 * An output example:
 * 
 * a is 0x5574283542a0 , *a is 0x5574283542c0
 * a + 1 is 0x5574283542a8, *(a+1) is 0x5574283542e0
 * (a[0]) is 10, a[0] is 0x5574283542c0, &a[0] is 0x5574283542a0
 * (a[1]) is 20, a[1] is 0x5574283542e0, &a[1] is 0x5574283542a8
 * a[1] is 20, &(*a)[1] is 0x5574283542c4
 * a[1] is 20, &(*a) + 1 is 0x5574283542a8
 * a[1] is 20, (int**)&(*a) + 1 is 0x5574283542a8
 * 
 */
