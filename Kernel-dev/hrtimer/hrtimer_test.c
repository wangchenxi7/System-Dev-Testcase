/**
 * hrtimer is the kernel level most high-resolution timer.
 * It's defined in include/linux/hrtimer.h
 * 
 * [?] Is this a count-down timer ?
 * 		Can use hrtimer to measure nanoseconds latency ?
 * 
 */

#include "hrtimer_test.h"
//#include "octopus_client.h"

MODULE_AUTHOR("Excavator,plsys");
MODULE_DESCRIPTION("HRTimer, nanoseconds level timer ");
MODULE_LICENSE("Dual BSD/GPL");  // This is necessary for importing some external symbols 
MODULE_VERSION("1.0");

struct hrtimer hr_timer;
unsigned cycles_low, cycles_high;
unsigned cycles_low_mid, cycles_high_mid;
unsigned cycles_low1, cycles_high1;
double cpu_freq = 2.6;

// expired time.
static unsigned long long completion_nsec = (unsigned long long)10000;  // expired time, ns


// [?] How to control the expired event ?
//		e.g. we recieved responds from remote Memory server, then stop the timer. 
static enum hrtimer_restart  hr_timer_expired(struct hrtimer *timer){
	uint64_t start, end;

	asm volatile("RDTSCP\n\t"
						"mov %%edx, %0\n\t"
						"mov %%eax, %1\n\t"
						"CPUID\n\t"
						: "=r"(cycles_high1), "=r"(cycles_low1)::"%rax", "%rbx", "%rcx", "%rdx");

	printk(" High-Resolution Timer expired.\n");
	

	start = (((uint64_t)cycles_high << 32) | cycles_low);
	end = (((uint64_t)cycles_high1 << 32) | cycles_low1);
	printk(KERN_INFO "init lat = %llu cycles\n", (end - start)  );

	// Just exit the timer.
	// one-shot timer which should not be started again
	return HRTIMER_NORESTART;
}





// Kernel functions can only be static ?
int __init timer_init(void){
	uint64_t start, end;
  printk("%s, Init self build kernel \n", __func__);

	//
	// CLOCK_MONOTONIC	: time increase from 0, nothing to do with wall-time.
	// HRTIMER_MODE_ABS : Absolute time value. 
	//struct hrtimer hr_timer;
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC,  HRTIMER_MODE_ABS);
	hr_timer.function	= hr_timer_expired;

	ktime_t hr_timer_expired = completion_nsec;
	

	// Read rdtscp #1
	asm volatile( "xorl %%eax, %%eax\n\t" 
							"CPUID\n\t"
							"RDTSCP\n\t"
							"mov %%edx, %0\n\t"           
							"mov %%eax, %1\n\t"            
							: "=r"(cycles_high), "=r"(cycles_low)::"%rax", "%rbx", "%rcx","%rdx");



	// Paramter
	//	1st, the clock we are using. CLOCK_MONOTONIC.
	//	2nd, the handler function when the setted time expired.
	//	3rd, the way we count time.
	// 			 HRTIMER_MODE_REL : mean that the expired time is relative to current time, the initiated time.
	hrtimer_start(&hr_timer, hr_timer_expired, HRTIMER_MODE_REL);  


	asm volatile("RDTSCP\n\t"
						"mov %%edx, %0\n\t"
						"mov %%eax, %1\n\t"
						"xorl %%eax, %%eax\n\t"
						"CPUID\n\t"
						: "=r"(cycles_high_mid), "=r"(cycles_low_mid)::"%rax", "%rbx", "%rcx", "%rdx");
	
	start = (((uint64_t)cycles_high << 32) | cycles_low);
	end = (((uint64_t)cycles_high_mid << 32) | cycles_low_mid);
	printk(KERN_INFO "init lat = %llu cycles\n", (end - start)  );


  return 0;
}


void __exit timer_exit(void){
  printk("Exit self build kernel \n");

  return;
}

module_init(timer_init);
module_exit(timer_exit);
