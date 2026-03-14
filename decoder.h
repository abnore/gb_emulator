#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include "cpu.h"
#include "bus.h"
/* I need a better way to decode the opcodes, instead of have a 500-step
  switch-case, which would have to be nested; or having 500 small helpers
  for each opcode. A lot is the same, differing only in operands.
  Take LD for example, its over 80 different operations which are all LD,
  with 8 and 16 bit operands, immediate and pointers

  We need to use the blocks instead, have 4 main groups, and decode it based
  on bits.

  The https://gbdev.io/pandocs/CPU_Instruction_Set.html tells us of the second
  block, which are all loads! That is 64 opcodes in one wrapper!

  opcode = ii xxx yyy
  ii is block, either 00 (block 0), 01 (1), 10 (2) or 11 (3).
  According to the table, block 1 is LD, and block 2 is ALU. block 0 is misc,
  and block 3, xxx and yyy is then indexes into those tables.


  So the system here (since they are in block0 (first two bits are 0), the last
  3 bits is which family of op codes, and then the middle 3 decide operands
    00 yyy zzz
    where zzz is checked first, then we decide internally based on yyy.
*/

void decode_block0(CPU *cpu, Bus *bus, uint8_t opcode);
void decode_block1(CPU *cpu, Bus *bus, uint8_t opcode);
void decode_block2(CPU *cpu, Bus *bus, uint8_t opcode);
void decode_block3(CPU *cpu, Bus *bus, uint8_t opcode);

void decoder(CPU *cpu, Bus *bus, uint8_t opcode);

#endif //DECODER_H
