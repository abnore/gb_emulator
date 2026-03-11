#include <blackbox.h>

#include "cpu.h"
#include "bus.h"


/* low 4 bits on flags register, and the flag itself, are always all 0,
 * except for when we unset on the flag (inverse)
 * Therefore we check if f is 0x10,0x20,0x40 or 0x80 to make sure, and
 * also reset the last bits on the register to be sure */

void set_flag(flag f, uint8_t *reg){
    if (f & 0xf) return;
    *reg |= (uint8_t)f;
    *reg &= 0xf0;
}
void unset_flag(flag f, uint8_t *reg){
    if (f & 0xf) return;
    *reg &= ~(uint8_t)f;
    *reg &= 0xf0;
}
int test_flag(flag f, uint8_t reg){
    return reg & f;
}

/* Passing the cpu and the rom everywhere seems tedious, and unecessary
 * I need a bus struct that can be at the centre of things */
static void op_00(CPU *cpu, Bus *bus){
    /* Maybe something later? cycle? */
    (void)cpu, (void)bus->rom;
}
static void op_c3(CPU *cpu, Bus *bus){
    byte lo = bus_read(bus, cpu->PC++);
    byte hi = bus_read(bus, cpu->PC++);
    cpu->PC = (hi << 8) | lo;
    DEBUG(": 0x%.4x", cpu->PC);
}
static void op_af(CPU *cpu, Bus *bus){
    cpu->A = 0;
    cpu->F = Z_F;
    (void)bus;
}
static void op_21(CPU *cpu, Bus *bus){
    cpu->L = bus_read(bus, cpu->PC++);
    cpu->H = bus_read(bus, cpu->PC++);
    DEBUG(": 0x%.4x", cpu->HL);
}

static void op_0e(CPU *cpu, Bus *bus){
    cpu->C = bus_read(bus,cpu->PC++);
    DEBUG(": 0x%.2x", cpu->C);
}
static void op_06(CPU *cpu, Bus *bus){
    cpu->B = bus_read(bus, cpu->PC++);
    DEBUG(": 0x%.2x", cpu->B);
}
static void op_32(CPU *cpu, Bus *bus){
    bus_write(bus, cpu->HL, cpu->A);
    cpu->HL--;
}

static void op_05(CPU *cpu, Bus *bus){
    byte old = cpu->B;
    cpu->B--;
    cpu->F =  (cpu->B ? 0 : Z_F)
            | N_F
            | ((old & 0x0F) == 0 ? H_F : 0)
            | (cpu->F & C_F);

    (void)bus;
}

Op optable[256] = {
    [0x00] = { "NOP",           4,      op_00 },
    [0x05] = { "DEC B",         4,      op_05 },
    [0x06] = { "LD B,n8",       8,      op_06 },
    [0x0e] = { "LD C,n8",       8,      op_0e },
    [0x21] = { "LD HL,d16",     12,     op_21 },
    [0x32] = { "LDD [HL],A",    8,      op_32 },
    [0xaf] = { "XOR A",         4,      op_af },
    [0xc3] = { "JP a16",        16,     op_c3 },
};

/* Simple, dont need more at this moment */
#define HALT() cpu->halted=true

/* Replacing the potentially 1k+ line switch case that was 62 lines after 6
 * op-codes already */
void cpu_step(CPU *cpu, Bus *bus)
{
    byte opcode = bus_read(bus, cpu->PC++);
    Op *op = &optable[opcode];

    if (!op->fn) {
        WARN("0x%.2x: ? [ opcode not implemented ]", opcode);
        HALT();
        return;
    }
    DEBUG("op: %s  ", op->name);
    op->fn(cpu, bus);
}
