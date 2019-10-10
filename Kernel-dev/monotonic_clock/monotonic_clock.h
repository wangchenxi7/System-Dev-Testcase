#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/init.h>
//#include <linux/posix-clock.h>
#include <linux/timekeeping.h>

//#include <linux/time.h>


//unsigned long long time_count_nsecs(void);



long timer_end(struct timespec start_time)
{
    struct timespec end_time;
    getrawmonotonic(&end_time);
    return(end_time.tv_nsec - start_time.tv_nsec);
}

struct timespec timer_start(void)
{
    struct timespec start_time;
    getrawmonotonic(&start_time);
    return start_time;
}