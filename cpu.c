#include <blackbox.h>

#include "cpu.h"
#include "opcodes.h"

/* The memory elements are two 8KiB registers, Work and Video RAM */
byte wram[0x2000];  // $C000-$DFFF in two 4k segments
byte hram[0x7f];     // $FF80-$FFFE
byte IE;            // $FFFF
byte IF;            // $FF0F

/* Memory Map
   The Game Boy has a 16-bit address bus, which is used to address ROM, RAM,
   and I/O.

   Start End Description
   0000 3FFF 16 KiB ROM bank 00	From cartridge, usually a fixed bank
   4000 7FFF 16 KiB ROM Bank 01–NN From cartridge, switchable bank via mapper
   A000 BFFF 8 KiB External RAM	From cartridge, switchable bank if any
   C000 CFFF 4 KiB Work RAM (WRAM)
   D000 DFFF 4 KiB Work RAM (WRAM)	In CGB mode, switchable bank 1–7
   FE00 FE9F Object attribute memory (OAM)
   FF00 FF7F I/O Registers
   FF80 FFFE High RAM (HRAM)
   FFFF FFFF Interrupt Enable register (IE)
*/

/* So far just get the next byte from the ROM, this will expand to cover the
 * entire memory map later */
byte read_bus(uint16_t address, ROM rom){
    (void)address;
    byte opcode = rom[address];
    return opcode;
}

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
static void op_00(CPU *cpu, ROM rom){
    /* Maybe something later? cycle? */
    (void)cpu, (void)rom;
}
static void op_c3(CPU *cpu, ROM rom){
    byte lo = rom[cpu->PC++];
    byte hi = rom[cpu->PC++];
    uint16_t address = (hi<<8) | lo;
    cpu->PC = address;
    DEBUG(": 0x%.4x", cpu->PC);
}
static void op_af(CPU *cpu, ROM rom){
    cpu->A = 0;
    (void)rom;
}
static void op_21(CPU *cpu, ROM rom){
    cpu->L = rom[cpu->PC++];
    cpu->H = rom[cpu->PC++];
    DEBUG(": 0x%.2x", cpu->HL);
}

static void op_0e(CPU *cpu, ROM rom){
    cpu->C = rom[cpu->PC++];
    DEBUG(": 0x%.2x", cpu->C);
}
static void op_06(CPU *cpu, ROM rom){
    cpu->B = rom[cpu->PC++];
    DEBUG(": 0x%.2x", cpu->B);
}
static void op_32(CPU *cpu, ROM rom){
    /* right now i need to manually offset this, since it's just a static
     * array from 0x0-0x2000, i dont have it mapped yet */
    wram[cpu->HL - 0xC000] = cpu->A;
    cpu->HL--;
    (void)rom;
}

Op optable[256] = {
    [0x00] = { "NOP",           4,      op_00 },
    [0xc3] = { "JP a16",        16,     op_c3 },
    [0xaf] = { "XOR A",         4,      op_af },
    [0x21] = { "LD HL,d16",     12,     op_21 },
    [0x0e] = { "LD C,n8",       8,      op_0e },
    [0x06] = { "LD B,n8",       8,      op_06 },
    [0x32] = { "LDD [HL],A",    8,      op_32 },
};

/* Simple, dont need more at this moment */
#define HALT() cpu->halted=true

/* Replacing the potentially 1k+ line switch case that was 62 lines after 6
 * op-codes already */
void cpu_step(CPU *cpu, ROM rom)
{
    byte opcode = read_bus(cpu->PC++, rom);
    Op *op = &optable[opcode];

    if (!op->fn) {
        WARN("0x%.2x: ? [ opcode not implemented ]\n", opcode);
        HALT();
        return;
    }
    DEBUG("op: %s  ", op->name);
    op->fn(cpu, rom);
}
