/**
 * Test the value of char.
 *  
 * 	char					: a character 
 *	signed char 	: -128 to 127
 * 	unsigned char : 0 -255
 * 
 */


#include "stdio.h"


unsigned char char_val(unsigned char val){
	printf("passed in char %c \n", val);
	printf("print as unsigned char 0x%u \n", val);
}

int main(int argc, char* argv[]){

	char_val(1);

	return 0;
}