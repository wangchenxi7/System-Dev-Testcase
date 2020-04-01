/**
 * struct pte_t {
 * 	usdinged long pte;
 * };
 * 
 * pte_t ptr * = some pte_t instace.
 * 
 * what's the difference between *ptr and ptr , when pass them as a parameter ?
 * 
 * 	pass ptr, works like reference parameter;
 *  pass *ptr, pass a copied value.
 * 
 */

#include "stdio.h"
#include "stdint.h"

struct pte_t{
	unsigned long pte;
};

void print_pte(struct pte_t pte){
	printf("pte_t 0x%llx,  val is 0x%llx \n", (uint64_t)&pte, (uint64_t)pte.pte);
}


int main(int argc, char* argv []){

	struct pte_t * ptr = (struct pte_t*)malloc(sizeof(struct pte_t));
	ptr->pte = 0x66;

	printf("pte pointer ptr 0x%llx, val 0x%llx \n", (uint64_t)ptr, (uint64_t)ptr->pte);
	
	print_pte(*ptr);

	return 0;
}