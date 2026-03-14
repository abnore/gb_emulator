#include <blackbox.h>

#include "cpu.h"
#include "bus.h"
#include "decoder.h"

/* Simple, dont need more at this moment */
#define HLT() cpu->halted=true

/* Replacing the potentially 1k+ line switch case that was 62 lines after 6
 * op-codes already */
void cpu_step(CPU *cpu, Bus *bus)
{
    printf("0x%.4x - ", cpu->PC);
    uint8_t opcode = bus_read(bus, cpu->PC++);
    decoder(cpu, bus, opcode);
}
