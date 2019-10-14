/**
 * nanoseconds level time counter based on RDTSC, RDTSCP instruction.
 * We has to write measurement function in assembly code.
 * 
 * Warning:
 * 1) lock the cpu frequency, it couts cpu cycles
 * 2) disable core preempt
 * 3) 64 bits, register. Not exceed the limitation.
 * 
 * 
 */


#include "rdtsc_clock.h"


double cpu_factor = 2.6; //2.6GHz

// Test the execution time of this function by rdtsc
void function_to_be_measured(int iteration){
	// int i;
	// for(i =0; i < iteration	; i++){
	// 	//printk("invocation %d \n",i);  // This function isn't statle.
	// }

	ndelay(1000);

}



int __init timer_init(void){
  printk("%s, Init self build kernel \n", __func__);

	unsigned cycles_low, cycles_high;
	unsigned cycles_low1, cycles_high1;
	unsigned long flags;

	preempt_disable();
	raw_local_irq_save(flags); /*we disable hard interrupts on our CPU*/

	//  1) rdtscp is used for solve  the Out-Of-Order interruptions.
	//	rdtscp can guarantee that the instructions before it are all finished, then start executing itself.
	//	But it can't guarantee that the instructions befind it not execute before rdtscp. 
	//	2) CPUID is a fence to precent Out-Of-Order ? 

/*
	asm volatile("xorl %%eax, %%eax\n\t"      
							"CPUID\n\t"
							"RDTSC\n\t"
							"mov %%edx, %0\n\t"           
							"mov %%eax, %1\n\t"            
							: "=r"(cycles_high), "=r"(cycles_low)::"%rax", "%rbx", "%rcx","%rdx");
*/


 asm volatile("xorl %%eax, %%eax\n\t"
              "CPUID\n\t"
              "RDTSC\n\t"
              "mov %%edx, %0\n\t"
              "mov %%eax, %1\n\t"
              : "=r"(cycles_high), "=r"(cycles_low)::"%rax", "%rbx", "%rcx",
                "%rdx");


//	function_to_be_measured(5);
	ndelay(1000);


/*
	asm volatile("RDTSCP\n\t"
						"mov %%edx, %0\n\t"
						"mov %%eax, %1\n\t"
						"xorl %%eax, %%eax\n\t"
						"CPUID\n\t"
						: "=r"(cycles_high1), "=r"(cycles_low1)::"%rax", "%rbx", "%rcx", "%rdx");
*/

asm volatile("RDTSCP\n\t"
              "mov %%edx, %0\n\t"
              "mov %%eax, %1\n\t"
              "xorl %%eax, %%eax\n\t"
              "CPUID\n\t"
              : "=r"(cycles_high1), "=r"(cycles_low1)::"%rax", "%rbx", "%rcx",
                "%rdx");

	raw_local_irq_restore(flags); // enable hard interrupts on this core
	preempt_enable();/* enable preemption*/

	uint64_t start, end;
	start = (((uint64_t)cycles_high << 32) | cycles_low);
	end = (((uint64_t)cycles_high1 << 32) | cycles_low1);
	printk(KERN_INFO "init lat = %llu cycles\n", (end - start) );


	return 0;
}



void __exit timer_exit(void){
  printk("Exit self build kernel \n");

  return;
}

module_init(timer_init);
module_exit(timer_exit);






