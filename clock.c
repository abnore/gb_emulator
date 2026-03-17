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
#include <blackbox.h>
#include "clock.h"

static mach_timebase_info_data_t timebase;
static uint64_t start_time;

void init_clock(void){
    kern_return_t result = mach_timebase_info(&timebase);
    if (result != KERN_SUCCESS) printf("i dunno... crash i guess\n");
    INFO("clock init");

    start_time = mach_absolute_time();
    //printf("denom: %i - numer %i\n", timebase.denom, timebase.numer);
}

/* We timestamp the start, then after a while we timestamp*/
int next_cycle(void){

    uint64_t t1 = mach_absolute_time();

    uint64_t elapsed_ns = (t1-start_time) * timebase.numer / timebase.denom;
    printf("[clock] elapsed ns: %llu\n", elapsed_ns);

    start_time = t1;

    uint64_t cycles_owed = elapsed_ns / 240;
    printf("[clock] cycles owed is: %llu\n", cycles_owed);
    return 0;
}
