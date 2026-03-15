#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include "gameboy.h"
/*
  I need a better way to decode the opcodes, instead of have a 500-step
  switch-case, which would have to be nested; or having 500 small helpers
  for each opcode. A lot is the same, differing only in operands.
  Take LD for example, its over 80 different operations which are all LD,
  with 8 and 16 bit operands, immediate and pointers

  The https://gbdev.io/pandocs/CPU_Instruction_Set.html explains:

  opcode = xx yyy zzz

  Sometimes p and q are used as y & 1 and y > 1, sometimes y is the index
*/

int decode_block0(Gameboy *gb, uint8_t opcode);
int decode_block1(Gameboy *gb, uint8_t opcode);
int decode_block2(Gameboy *gb, uint8_t opcode);
int decode_block3(Gameboy *gb, uint8_t opcode);

int decoder(Gameboy *gb, uint8_t opcode);

/* Named enums such that i dont have to rely on magic index numbers in the
 * decoder and helper functions. I am switching on indexes, not actual values
 * therefore i need a system to differentiate. And they are always in the same
 * order, this enabled this to work
 */
typedef enum {
    R8_B,
    R8_C,
    R8_D,
    R8_E,
    R8_H,
    R8_L,
    R8_HL,
    R8_A,
} r8_e;

typedef enum {
    R16_BC,
    R16_DE,
    R16_HL,
    R16_SP,
} r16_e;

typedef enum {
    R16STK_BC,
    R16STK_DE,
    R16STK_HL,
    R16STK_AF,
} r16stk_e;

typedef enum {
    CC_NZ,
    CC_Z,
    CC_NC,
    CC_C,
} cc_e;

typedef enum {
    R16MEM_BC,
    R16MEM_DE,
    R16MEM_HL_INC,
    R16MEM_HL_DEC,
} r16mem_e;

#endif //DECODER_H
