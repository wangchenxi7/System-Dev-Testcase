/**
 * 
 * Output:
 * 	test_short_1 : 2 bytes
 *	test_short_2 : 4 bytes
 *	test_int_1 : 4 bytes
 * 
 * 
 *  Explanation:
 * 
 *  for short_bit_field_1, at least 1 short, 2  bytes;
 *  for short_bit_field_2, it needs 2 short to store 17 bits;
 *  for int_bit_field_1, it needs at least 1 int, 4 bytes;
 *  
 */


#include "stdio.h"

struct short_bit_field_1{
	unsigned short a : 3;
	unsigned short b : 4;
} test_short_1;

struct short_bit_field_2{
	unsigned short a : 3;
	unsigned short b : 14;
} test_short_2;

struct int_bit_field_1{
	unsigned int a : 3;
	unsigned int b : 4;
} test_int_1;


int main(int argc, char* argv[]){


	printf("test_short_1 : %lu bytes \n", sizeof(test_short_1));
	printf("test_short_2 : %lu bytes \n", sizeof(test_short_2));
	printf("test_int_1 : %lu bytes \n", sizeof(test_int_1));

	return 0;
}
