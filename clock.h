#ifndef CLOCK_H_
#define CLOCK_H_

#include <mach/mach_time.h>

#define MCLOCK 4194304      // Hz (4.194304 MHz)
#define SCLOCK	MCLOCK>>2   // 1/4 av the master clock

void init_clock(void);
int next_cycle();

#endif //CLOCK_H_
