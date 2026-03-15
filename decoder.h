#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include <stdbool.h>
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


/* Helper functions below based on the functions needed for the decoding */

/* Reading the byte the PC is pointing at as an immediate, and moving PC */
uint8_t read_imm8(Gameboy *gb);
/* Reading two bytes, little endian, and return a 16bit value */
uint16_t read_imm16(Gameboy *gb);
/* Read from a 8bit register based on index: B,C,D,E,H,L,[HL],A,
 * notigce [HL] is the byte at the address in HL */
uint8_t read_r8(Gameboy *gb, r8_e idx);
/* Same as above, only writing to instead of reading from */
void write_r8(Gameboy *gb, r8_e idx, uint8_t value);
/* Read an operand from a 16bit register selected by index: BC,DE,HL,SP */
uint16_t read_r16(Gameboy *gb, r16_e idx);
/* Same as above, only writing */
void write_r16(Gameboy *gb, r16_e idx, uint16_t value);
/* Reading a 16bit stack operand based on index: BC,DE,HL,AF.
 * Separate from r16 because stack instructions use AF instead of SP.
 * Need a seperate indexing scheme since the operand family is different,
 * and therefore they couldnt share a table even though 0-2 is the same */
uint16_t read_r16stk(Gameboy *gb, r16stk_e idx);
/* Same only writing */
void write_r16stk(Gameboy *gb, r16stk_e idx, uint16_t value);
/* Returns the address pointed to by [BC] and [DE], also
 * [HL] with an increment and decrement */
uint16_t addr_r16(Gameboy *gb, r16mem_e op);
void step_r16(Gameboy *gb, r16mem_e op);
/* Pushes a 16bit value on the stack, first decrements SP and then write high
 * byte first */
void push16(Gameboy *gb, uint16_t value) ;
/* Pops a 16bit value from the stack, and updating the SP */
uint16_t pop16(Gameboy *gb) ;
bool cc_true(Gameboy *gb, cc_e cc);
/* LD using normal r8 and r16 operands and imm.
 * The first one is all block 1 needs to work, the 3 under as block 0 helpers */
void ld_r8_r8(Gameboy *gb, r8_e dst, r8_e src);
void ld_r8_imm8(Gameboy *gb, r8_e dst);
void ld_r16_imm16(Gameboy *gb, r16_e dst);
void ld_mem_hl_imm8(Gameboy *gb);
/* Uses [BC], [DE] and [HL+] [HL-] */
void ld_mem_r16_a(Gameboy *gb, r16mem_e dst);
void ld_a_mem_r16(Gameboy *gb, r16mem_e src);
/* LD  using an imm16bit address operand, a16, first two are a store X a16 */
/* First the LD [a16], SP. We need to interprete the a16 as an address, then
 * write the contents of SP to that address, in little endian, since SP is 16bit
 * we write 2 bytes, starting from the address */
void ld_a16_sp(Gameboy *gb);
/* This does the same, only now from A, we store only 1 byte
 * LD [a16], A */
void ld_a16_a(Gameboy *gb);
/* LD A, [a16] */
void ld_a_a16(Gameboy *gb);
/* LD SP, HL and the special LD HL, SP+e8 form */
void ld_sp_hl(Gameboy *gb);
/* Add the signed value e8 to SP and copy the result in HL. This instruction
 * sets flags: Z and N is set to 0, and H Set if overflow from bit 3. C Set if
 * overflow from bit 7. */
void ld_hl_sp_e8(Gameboy *gb);
/* Increments and decrements r8 and r16 register, also sets flags
 * Z Set if result is 0.
 * N 0
 * H Set if overflow from bit 3. */
void inc_r8(Gameboy *gb, r8_e idx);
/* Z Set if result is 0.
 * N 1
 * H Set if borrow from bit 4.*/
void dec_r8(Gameboy *gb, r8_e idx);
/* Increments and decrements a 16bit register, but flags are untouched */
void inc_r16(Gameboy *gb, r16_e idx);
void dec_r16(Gameboy *gb, r16_e idx);

/* special add operation
 * Add the value in r16 to HL.
 * N 0
 * H Set if overflow from bit 11.
 * C Set if overflow from bit 15.*/
void add_hl_r16(Gameboy *gb, r16_e idx);
void add_sp_e8(Gameboy *gb);
/* ALU imm8 to A */
void add_a_imm8(Gameboy *gb);
void adc_a_imm8(Gameboy *gb);
void sub_a_imm8(Gameboy *gb);
void sbc_a_imm8(Gameboy *gb);
void and_a_imm8(Gameboy *gb);
void xor_a_imm8(Gameboy *gb);
void or_a_imm8(Gameboy *gb);
void cp_a_imm8(Gameboy *gb);
/* ALU operations for block 2 - r8 to A */
void add_a(Gameboy *gb, r8_e idx);
void adc_a(Gameboy *gb, r8_e idx);
void sub_a(Gameboy *gb, r8_e idx);
void sbc_a(Gameboy *gb, r8_e idx);
void and_a(Gameboy *gb, r8_e idx);
void xor_a(Gameboy *gb, r8_e idx);
void or_a (Gameboy *gb, r8_e idx);
void cp_a (Gameboy *gb, r8_e idx);

/* Jumping to a absolute n16 address */
void jmp_n16(Gameboy *gb);

#endif //DECODER_H
