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
int cb_dispatch(Gameboy *gb);

int decoder(Gameboy *gb, uint8_t opcode);

#define HLT 0x76
#define HARDLOCK() for(;;)
#define unreachable() __builtin_unreachable()

/* The flags register, F, contains 4 flags in bit 7-4
 * https://gbdev.io/pandocs/CPU_Registers_and_Flags.html
 */
typedef enum{
    Z_F = 1<<7, // Zero flag
    N_F = 1<<6, // Subtraction flag (BCD)
    H_F = 1<<5, // Half Carry flag (BCD)
    C_F = 1<<4, // Carry flag
} flag;

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

/*
bool test_flag(Gameboy *gb, flag f);
void set_flag(Gameboy *gb, flag f);
void clear_flag(Gameboy *gb, flag f);
void assign_flag(Gameboy *gb, flag f, bool cond);
void clear_znhc(Gameboy *gb);
void keep_c_clear_zhn(Gameboy *gb);
*/

/* Core helpers, operand machinery */
uint8_t read_r8(Gameboy *gb, r8_e idx);
uint8_t read_imm8(Gameboy *gb);
uint16_t read_r16(Gameboy *gb, r16_e idx);
uint16_t read_imm16(Gameboy *gb);
uint16_t read_r16stk(Gameboy *gb, r16stk_e idx);
void write_r8(Gameboy *gb, r8_e idx, uint8_t value);
void write_r16(Gameboy *gb, r16_e idx, uint16_t value);
void write_r16stk(Gameboy *gb, r16stk_e idx, uint16_t value);
uint16_t addr_r16(Gameboy *gb, r16mem_e op);
void step_r16(Gameboy *gb, r16mem_e op);
bool cc_true(Gameboy *gb, cc_e cc);

/* Load Instructions */
void ld_r8_r8(Gameboy *gb, r8_e dst, r8_e src);
void ld_r8_imm8(Gameboy *gb, r8_e dst);
void ld_r16_imm16(Gameboy *gb, r16_e dst);
void ld_mem_r16_a(Gameboy *gb, r16mem_e dst);
void ld_mem_a16_a(Gameboy *gb);
void ldh_mem_a8_a(Gameboy *gb);
void ldh_mem_c_a(Gameboy *gb);
void ld_a_mem_r16(Gameboy *gb, r16mem_e src);
void ld_a_mem_a16(Gameboy *gb);
void ldh_a_mem_a8(Gameboy *gb);
void ldh_a_mem_c(Gameboy *gb);

/* 8-bit arithmetic instructions */
void adc_a(Gameboy *gb, r8_e idx);
void adc_a_imm8(Gameboy *gb);
void add_a(Gameboy *gb, r8_e idx);
void add_a_imm8(Gameboy *gb);
void cp_a (Gameboy *gb, r8_e idx);
void cp_a_imm8(Gameboy *gb);
void dec_r8(Gameboy *gb, r8_e idx);
void inc_r8(Gameboy *gb, r8_e idx);
void sbc_a(Gameboy *gb, r8_e idx);
void sbc_a_imm8(Gameboy *gb);
void sub_a(Gameboy *gb, r8_e idx);
void sub_a_imm8(Gameboy *gb);

/* 16-bit arithmetic instructions */
void add_hl_r16(Gameboy *gb, r16_e idx);
void dec_r16(Gameboy *gb, r16_e idx);
void inc_r16(Gameboy *gb, r16_e idx);

/* Bitwise logic instructions */
void and_a(Gameboy *gb, r8_e idx);
void and_a_imm8(Gameboy *gb);
void cpl(Gameboy *gb);
void or_a (Gameboy *gb, r8_e idx);
void or_a_imm8(Gameboy *gb);
void xor_a(Gameboy *gb, r8_e idx);
void xor_a_imm8(Gameboy *gb);

/* Bit flag instructions */
void bit_u3_r8(Gameboy *gb, uint8_t u3, r8_e idx);
void res_u3_r8(Gameboy *gb, uint8_t u3, r8_e idx);
void set_u3_r8(Gameboy *gb, uint8_t u3, r8_e idx);

/* Bit shift instructions */
void rl(Gameboy *gb, r8_e idx);
void rla(Gameboy *gb);
void rlc(Gameboy *gb, r8_e idx);
void rlca(Gameboy *gb);
void rr(Gameboy *gb, r8_e idx);
void rra(Gameboy *gb);
void rrc(Gameboy *gb, r8_e idx);
void rrca(Gameboy *gb);
void sla(Gameboy *gb, r8_e idx);
void sra(Gameboy *gb, r8_e idx);
void srl(Gameboy *gb, r8_e idx);
void swap(Gameboy *gb, r8_e idx);

/* Jumps and subroutine instructions */
void call_a16(Gameboy *gb);
bool call_cc_a16(Gameboy *gb, cc_e cc);
void jp_a16(Gameboy *gb);
void jp_hl(Gameboy *gb);
bool jr_cc_e8(Gameboy *gb, cc_e cc);
void jr_e8(Gameboy *gb);
bool jp_cc_a16(Gameboy *gb, cc_e cc);
void ret(Gameboy *gb);
bool ret_cc(Gameboy *gb, cc_e cc);
void reti(Gameboy *gb);
void rst(Gameboy *gb, uint8_t address);

/* Carry flag instructions */
void ccf(Gameboy *gb);
void scf(Gameboy *gb);

/* Stack manipulation instructions */
void add_sp_e8(Gameboy *gb);
void ld_sp_hl(Gameboy *gb);
void ld_hl_sp_e8(Gameboy *gb);
void ld_mem_a16_sp(Gameboy *gb);
void push16(Gameboy *gb, uint16_t value) ;
uint16_t pop16(Gameboy *gb) ;

/* Interrupt-related instructions */
void ei(Gameboy *gb);
void di(Gameboy *gb);
void halt(Gameboy *gb); // TODO: IME flag and interrupts needs to be handled

/* Miscellaneous instructions */
void daa(Gameboy *gb);
void nop(void);
void stop(Gameboy *gb);

#endif //DECODER_H
