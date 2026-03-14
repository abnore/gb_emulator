#ifndef GB_H
#define GB_H

#include "cpu.h"
#include "bus.h"

typedef struct Gameboy{
    CPU cpu;
    Bus bus;
} Gameboy;

#endif // GB_H
