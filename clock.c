#include <blackbox.h>
#include "clock.h"


/*
kern_return_t is literally just an int

struct mach_timebase_info {
    uint32_t numer;
    uint32_t denom;
};

nanoseconds = ticks * numer / denom
on my system numer is 125 and denom is 3

So you can do:

uint64_t t0 = mach_absolute_time();
uint64_t t1 = mach_absolute_time();
uint64_t dt = t1 - t0;

Then elapsed nanoseconds is dt * 125/3
To get seconds ( which i dont plan on ) i need to take that and  / 1e9
*/
static mach_timebase_info_data_t timebase;
static uint64_t start_time;

void init_clock(void){
    kern_return_t result = mach_timebase_info(&timebase);
    if (result != KERN_SUCCESS) printf("OWO\n");
    INFO("clock init");

    start_time = mach_absolute_time();
    printf("denom: %i - numer %i\n", timebase.denom, timebase.numer);
}

/* This does not work -
 * 17:47:30 [TRACE] clock.c:next_cycle():42 => elapsed ns: 15583
 * A simple print takes more time then the ~240ns i want. I need to think of a
 * different solution
 */
int next_cycle(){

    uint64_t t1 = mach_absolute_time();

    uint64_t ens = (t1-start_time) * timebase.numer / timebase.denom;
    TRACE("elapsed ns: %llu", ens);

    if (ens > 240){
        start_time = t1;
        return 1;
    }

    return 0;
}


