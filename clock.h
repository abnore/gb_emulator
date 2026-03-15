#ifndef CLOCK_H_
#define CLOCK_H_

#include <mach/mach_time.h>

/* for now i just store the cycles in the Gameboy object, but i might need
 * this later. There are a lot of things depending on the clock.
 */
typedef struct {
    uint64_t cycles;
//    uint64_t t0;
//    uint64_t t1;
} Clock;

#define MCLOCK  1<<22       // 4.194304 MHz or 2^22
#define SCLOCK	MCLOCK>>2   // 1.048576 or 1/4 av the master clock

void init_clock(void);
int next_cycle();

#endif //CLOCK_H_
