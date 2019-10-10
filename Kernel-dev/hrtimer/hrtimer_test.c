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

// expired time.
static unsigned long long completion_nsec = (unsigned long long)10000000000;  // expired time, ns


// [?] How to control the expired event ?
//		e.g. we recieved responds from remote Memory server, then stop the timer. 
static enum hrtimer_restart  hr_timer_expired(struct hrtimer *timer){

	printk(" High-Resolution Timer expired.\n");

	// Just exit the timer.
	// one-shot timer which should not be started again
	return HRTIMER_NORESTART;
}





// Kernel functions can only be static ?
int __init timer_init(void){
  printk("%s, Init self build kernel \n", __func__);

	//
	// CLOCK_MONOTONIC	: time increase from 0, nothing to do with wall-time.
	// HRTIMER_MODE_ABS : Absolute time value. 
	//struct hrtimer hr_timer;
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC,  HRTIMER_MODE_ABS);
	hr_timer.function	= hr_timer_expired;

	ktime_t hr_timer_expired = completion_nsec;
	
	// Paramter
	//	1st, the clock we are using. CLOCK_MONOTONIC.
	//	2nd, the handler function when the setted time expired.
	//	3rd, the way we count time.
	// 			 HRTIMER_MODE_REL : mean that the expired time is relative to current time, the initiated time.
	hrtimer_start(&hr_timer, hr_timer_expired, HRTIMER_MODE_REL);  

  return 0;
}


void __exit timer_exit(void){
  printk("Exit self build kernel \n");

  return;
}

module_init(timer_init);
module_exit(timer_exit);
