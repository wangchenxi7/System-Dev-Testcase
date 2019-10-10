/**
 * Test the ns level timer counter.
 * 
 * Warning : the timespec is defined in, include/linux/time.h.
 * struct timespec {
 *		__kernel_time_t	tv_sec;			// seconds
 *		long		tv_nsec;						// nanoseconds
 * };
 * 
 * timespec.tv_nsec is less than 1s. 
 * 
 * [Aborted] This will incur 1-3 us overhead
 * 
 * 
 */


#include "monotonic_clock.h"
//#include "octopus_client.h"

MODULE_AUTHOR("Excavator,plsys");
MODULE_DESCRIPTION("MONOTONIC CLOCK, nanoseconds level clock ");
MODULE_LICENSE("Dual BSD/GPL");  // This is necessary for importing some external symbols 
MODULE_VERSION("1.0");


//
// Global time counter
//
struct timespec start_time;


//
// Compare the time in hrtimer  and clock_gettime()
//
struct hrtimer hr_timer;
// expired time.
static unsigned long long completion_nsec = (unsigned long long)10000;  // expired time, count at ns. 10 microsecons



// [?] How to control the expired event ?
//		e.g. we recieved responds from remote Memory server, then stop the timer. 
static enum hrtimer_restart  hr_timer_expired(struct hrtimer *timer){

	long elapsed_time = timer_end(start_time);
	// calculate the end time.
	printk(" High-Resolution Timer expired at time %lu ns.\n", elapsed_time );

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
	//start_time = time_count_nsecs();  // record the start time

	//start_time = timer_start();
	hrtimer_start(&hr_timer, hr_timer_expired, HRTIMER_MODE_REL);  
	start_time = timer_start();

  return 0;
}


void __exit timer_exit(void){
  printk("Exit self build kernel \n");

  return;
}

module_init(timer_init);
module_exit(timer_exit);
