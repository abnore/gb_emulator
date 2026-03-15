/* Main decoder function
 *
 * We will switch on the opcode block - either 0,1,2 or 3 based on first 2 bits.
 *
 * The logic will be based on a fixed ordering on the registers internally.
 * When its r16 it is always BC, DE, HL, SP/AF for stack operations
 * when its r8 it is always  B, C, D, E, H, L, [HL], A
 * Enums created for this: r8_e, r16_e, r16stk_e, r16mem_e
 *
 * Block 0:
 *   Mixed block. First split on z (low 3 bits).
 *   z=0: specials sorted on y
 *   z=1,2,3: paired families using q, with p selecting operands
 *   z=4,5,6,7: direct families sorted on y
 *
 * Block 1:
 *   LD r8, r8 matrix.
 *
 * Block 2:
 *   ALU operations always on A.
 *
 * Block 3:
 *   Control flow, stack, immediates, misc.
 *
 * Helpers i have identified sorting all 4 blocks, some here MAY be compressed
 * and others may be wrappers of eachother, and some need the bus, but most
 * use the rom and cpu directly
 *
 * Implemented:
 *  read_imm8
 *  read_imm16
 *
 *  read_r8
 *  write_r8
 *  read_r16
 *  write_r16
 *  read_r16stk
 *  write_r16stk
 *  addr_r16
 *  step_r16
 *
 *  push16
 *  pop16
 *  cc_true
 *
 *  ld_r8_r8
 *  ld_r8_imm8
 *  ld_r16_imm16
 *  ld_mem_hl_imm8
 *  ld_mem_r16_a
 *  ld_a_mem_r16
 *  ld_a16_sp
 *  ld_a16_a
 *  ld_a_a16
 *  ld_sp_hl
 *  ld_hl_sp_e8
 *  ldh_mem_a8_a
 *  ldh_a_mem_a8
 *
 *  inc_r8
 *  dec_r8
 *  inc_r16
 *  dec_r16
 *  add_hl_r16
 *  add_sp_e8

 *  add_a / imm8
 *  adc_a / imm8
 *  sub_a / imm8
 *  sbc_a / imm8
 *  and_a / imm8
 *  xor_a / imm8
 *  or_a  / imm8
 *  cp_a  / imm8
 *
 *  halt - TODO: IME flag and interrupts needs to be handled
 *  stop
 *  di
 *  ei
 *
 *  jmp_a16
 *  jp_hl
 *  jr_cc_e8
 *  jr_e8
 *  jp_cc_a16
 *
 *  rlca
 *  rrca
 *  rla
 *  rra
 *
 * TODO:
 *  daa
 *  cpl
 *  scf
 *  ccf
 *
 *  call_a16
 *  call_cc_a16
 *  ret
 *  ret_cc
 *  reti
 *  rst
 *
 *  cb_dispatch
 *  rot_r8
 *  bit_r8
 *  res_r8
 *  set_r8


n8 means immediate 8-bit data
n16 means immediate little-endian 16-bit data
a8 means 8-bit unsigned data, which is added to $FF00 in certain instructions
    to create a 16-bit address in HRAM (High RAM)
a16 means little-endian 16-bit address
e8 means 8-bit signed data

LDH A, [C] has the alternative mnemonic LD A, [$FF00+C]
LDH [C], A has the alternative mnemonic LD [$FF00+C], A
LD A, [HL+] has the alternative mnemonics LD A, [HLI] and LDI A, [HL]
LD [HL+], A has the alternative mnemonics LD [HLI], A and LDI [HL], A
LD A, [HL-] has the alternative mnemonics LD A, [HLD] and LDD A, [HL]
LD [HL-], A has the alternative mnemonics LD [HLD], A and LDD [HL], A

ALU instructions (ADD, ADC, SUB, SBC, AND, XOR, OR, and CP) can be written with
the left-hand side A omitted. Thus for example ADD A, B has the alternative
mnemonic ADD B, and CP A, $F has the alternative mnemonic CP $F.

Z - Zero Flag
N - Subtract Flag
H - Half Carry Flag
C - Carry Flag
0 - The flag is reset
1 - The flag is set
- - The flag is left untouched

If an operation has the flags defined as Z, N, H, or C, the corresponding flags
are set as the operation performed dictates.
*/

#include <blackbox.h>
#include <stdbool.h>

#include "decoder.h"

#define HLT 0x76

/* Decodes the op code and returns the amount of cycles it spent */
int decoder(Gameboy *gb, uint8_t opcode)
{
    uint8_t block = opcode >> 6;
    switch (block) {
    case 0: return decode_block0(gb, opcode);
    case 1: return decode_block1(gb, opcode);
    case 2: return decode_block2(gb, opcode);
    case 3: return decode_block3(gb, opcode);
    }
    return 0;
}

/* Helper functions below based on the functions needed for the decoding */
/* Reading the byte the PC is pointing at as an immediate, and moving PC */
uint8_t read_imm8(Gameboy *gb){
    return bus_read(gb, gb->cpu.PC++);
}
/* Reading two bytes, little endian, and return a 16bit value */
uint16_t read_imm16(Gameboy *gb){
    uint8_t lo = read_imm8(gb);
    uint8_t hi = read_imm8(gb);
    return ((uint16_t)hi << 8) | lo;
}
/* Read from a 8bit register based on index: B,C,D,E,H,L,[HL],A,
 * notice [HL] is the byte at the address in HL */
uint8_t read_r8(Gameboy *gb, r8_e idx){
    switch (idx) {
    case R8_B:  return gb->cpu.B;
    case R8_C:  return gb->cpu.C;
    case R8_D:  return gb->cpu.D;
    case R8_E:  return gb->cpu.E;
    case R8_H:  return gb->cpu.H;
    case R8_L:  return gb->cpu.L;
    case R8_HL: return bus_read(gb, gb->cpu.HL);
    case R8_A:  return gb->cpu.A;
    }
    return 0xff;
}
/* Same as above, only writing to instead of reading from */
void write_r8(Gameboy *gb, r8_e idx, uint8_t value){
    switch (idx) {
    case R8_B:  gb->cpu.B = value; break;
    case R8_C:  gb->cpu.C = value; break;
    case R8_D:  gb->cpu.D = value; break;
    case R8_E:  gb->cpu.E = value; break;
    case R8_H:  gb->cpu.H = value; break;
    case R8_L:  gb->cpu.L = value; break;
    case R8_HL: bus_write(gb, gb->cpu.HL, value); break;
    case R8_A:  gb->cpu.A = value; break;
    }
}
/* Read an operand from a 16bit register selected by index: BC,DE,HL,SP */
uint16_t read_r16(Gameboy *gb, r16_e idx){
    switch (idx) {
    case R16_BC:  return gb->cpu.BC;
    case R16_DE:  return gb->cpu.DE;
    case R16_HL:  return gb->cpu.HL;
    case R16_SP:  return gb->cpu.SP;
    }
    return 0xffff;
}
/* Same as above, only writing */
void write_r16(Gameboy *gb, r16_e idx, uint16_t value){
    switch (idx) {
    case R16_BC: gb->cpu.BC = value; break;
    case R16_DE: gb->cpu.DE = value; break;
    case R16_HL: gb->cpu.HL = value; break;
    case R16_SP: gb->cpu.SP = value; break;
    }
}
/* Reading a 16bit stack operand based on index: BC,DE,HL,AF.
 * Separate from r16 because stack instructions use AF instead of SP.
 * Need a seperate indexing scheme since the operand family is different,
 * and therefore they couldnt share a table even though 0-2 is the same */

uint16_t read_r16stk(Gameboy *gb, r16stk_e idx){
    switch (idx) {
    case R16STK_BC:  return gb->cpu.BC;
    case R16STK_DE:  return gb->cpu.DE;
    case R16STK_HL:  return gb->cpu.HL;
    case R16STK_AF:  return gb->cpu.AF;
    }
    return 0xffff;
}
/* Same only writing */
void write_r16stk(Gameboy *gb, r16stk_e idx, uint16_t value){
    switch (idx) {
    case R16STK_BC: gb->cpu.BC = value; break;
    case R16STK_DE: gb->cpu.DE = value; break;
    case R16STK_HL: gb->cpu.HL = value; break;
    case R16STK_AF: gb->cpu.AF = value; break;
    }
}

/* Returns the address pointed to by [BC] and [DE], also
 * [HL] with an increment and decrement */
uint16_t addr_r16(Gameboy *gb, r16mem_e op){
    switch (op) {
    case R16MEM_BC:     return gb->cpu.BC;
    case R16MEM_DE:     return gb->cpu.DE;
    case R16MEM_HL_INC: return gb->cpu.HL;
    case R16MEM_HL_DEC: return gb->cpu.HL;
    }
    return 0xffff;
}
void step_r16(Gameboy *gb, r16mem_e op){
    switch (op) {
    case R16MEM_HL_INC: gb->cpu.HL++; break;
    case R16MEM_HL_DEC: gb->cpu.HL--; break;
    default: break; // Not stepping BC and DE
    }
}
/* Pushes a 16bit value on the stack, first decrements SP and then write high
 * byte first */
void push16(Gameboy *gb, uint16_t value) {
    bus_write(gb, --(gb->cpu.SP), (uint8_t)(value >> 8));
    bus_write(gb, --(gb->cpu.SP), (uint8_t)value);
}
/* Pops a 16bit value from the stack, and updating the SP */
uint16_t pop16(Gameboy *gb) {
    uint8_t lo = bus_read(gb, gb->cpu.SP++);
    uint8_t hi = bus_read(gb, gb->cpu.SP++);
    return ((uint16_t)hi << 8) | lo;
}
bool cc_true(Gameboy *gb, cc_e cc){
    uint8_t flag = gb->cpu.F;
    switch (cc) {
    case CC_NZ: return (flag & Z_F) == 0;
    case CC_Z:  return (flag & Z_F) != 0;
    case CC_NC: return (flag & C_F) == 0;
    case CC_C:  return (flag & C_F) != 0;
    };
    return false;
}

/* LD using normal r8 and r16 operands and imm.
 * The first one is all block 1 needs to work, the 3 under as block 0 helpers */
void ld_r8_r8(Gameboy *gb, r8_e dst, r8_e src){
    write_r8(gb, dst, read_r8(gb, src));
}
void ld_r8_imm8(Gameboy *gb, r8_e dst){
    uint8_t val = read_imm8(gb);
    printf("0x%x - dst:%d ", val, dst);
    write_r8(gb, dst,val);
}
void ld_r16_imm16(Gameboy *gb, r16_e dst){
    uint16_t val = read_imm16(gb);
    printf("0x%x dst:%d- ",val, dst);
    write_r16(gb, dst,val);
}

/* Uses [BC], [DE] and [HL+] [HL-] both ways, to and from A*/
void ld_mem_r16_a(Gameboy *gb, r16mem_e dst){
    printf("ld_mem_r16: A into %d - 0x%x\n",dst, addr_r16(gb, dst));
    bus_write(gb, addr_r16(gb, dst), gb->cpu.A);
    step_r16(gb, dst);
}
void ld_a_mem_r16(Gameboy *gb, r16mem_e src){
    write_r8(gb, R8_A, bus_read(gb, addr_r16(gb, src)));
    step_r16(gb, src);
}

/* LD  using an imm16bit address operand, a16, first two are a store X a16 */
/* First the LD [a16], SP. We need to interprete the a16 as an address, then
 * write the contents of SP to that address, in little endian, since SP is 16bit
 * we write 2 bytes, starting from the address */
void ld_a16_sp(Gameboy *gb){
    uint16_t addr = read_imm16(gb);
    bus_write(gb, addr,     (uint8_t)gb->cpu.SP);
    bus_write(gb, addr + 1, (uint8_t)(gb->cpu.SP >> 8));
}
/* This does the same, only now from A, we store only 1 byte
 * LD [a16], A */
void ld_a16_a(Gameboy *gb){
    uint16_t addr = read_imm16(gb);
    bus_write(gb, addr, gb->cpu.A);
}
/* LD A, [a16] */
void ld_a_a16(Gameboy *gb){
    uint16_t addr = read_imm16(gb);
    write_r8(gb, R8_A, bus_read(gb, addr));
}
/* LD SP, HL and the special LD HL, SP+e8 form */
void ld_sp_hl(Gameboy *gb){
    write_r16(gb, R16_SP, read_r16(gb, R16_HL));
}
/* Add the signed value e8 to SP and copy the result in HL.
 * flags:
 * Z 0
 * N 0
 * H Set if overflow from bit 3.
 * C Set if overflow from bit 7. */
void ld_hl_sp_e8(Gameboy *gb){
    uint16_t sp = read_r16(gb, R16_SP);
    int8_t e8 = (int8_t)read_imm8(gb);
    uint16_t result = sp + e8;
    gb->cpu.F = 0;

    if (((sp & 0x0F) + ((uint8_t)e8 & 0x0F)) > 0x0F) gb->cpu.F |= H_F;
    if (((sp & 0xFF) + ((uint8_t)e8 & 0xFF)) > 0xFF) gb->cpu.F |= C_F;

    write_r16(gb, R16_HL, result);
}

void ldh_mem_a8_a(Gameboy *gb){
    uint16_t addr = 0xFF00 + read_imm8(gb);
    bus_write(gb, addr, gb->cpu.A);
}

void ldh_a_mem_a8(Gameboy *gb){
    uint8_t a8 = read_imm8(gb);
    uint16_t addr = 0xFF00 + a8;
    uint8_t value = bus_read(gb, addr);

    printf("LDH A,[%02x] addr=%04x value=%02x\n", a8, addr, value);

    write_r8(gb, R8_A, value);
}
void ldh_c_a(Gameboy *gb){
    uint16_t addr = 0xFF00 + gb->cpu.C;
    bus_write(gb, addr, gb->cpu.A);
}

void ldh_a_c(Gameboy *gb){
    uint16_t addr = 0xFF00 + gb->cpu.C;
    write_r8(gb, R8_A, bus_read(gb, addr));
}
/* Increments and decrements r8 and r16 register, also sets flags
 * Z Set if result is 0.
 * N 0
 * H Set if overflow from bit 3. */
void inc_r8(Gameboy *gb, r8_e idx){
    uint8_t old = read_r8(gb, idx);
    uint8_t value = old + 1;

    gb->cpu.F &= C_F;
    if (value == 0)             gb->cpu.F |= Z_F;
    if ((old & 0x0F) == 0x0F)   gb->cpu.F |= H_F;

    write_r8(gb, idx, value);
}
/* Z Set if result is 0.
 * N 1
 * H Set if borrow from bit 4.*/
void dec_r8(Gameboy *gb, r8_e idx){
    uint8_t old = read_r8(gb, idx);
    uint8_t value = old - 1;

    gb->cpu.F &= C_F;
    gb->cpu.F |= N_F;
    if (value == 0)             gb->cpu.F |= Z_F;
    if ((old & 0x0F) == 0x00)   gb->cpu.F |= H_F;

    printf("dec r8 idx:%d value:%d\n", idx, value);
    write_r8(gb, idx, value);
}
/* Increments and decrements a 16bit register, but flags are untouched */
void inc_r16(Gameboy *gb, r16_e idx){
    write_r16(gb, idx, read_r16(gb, idx) + 1);
}
void dec_r16(Gameboy *gb, r16_e idx){
    write_r16(gb, idx, read_r16(gb, idx) - 1);
}
/* special add operation
 * Add the value in r16 to HL.
 * N 0
 * H Set if overflow from bit 11. 0000 0000 0000 0000
 * C Set if overflow from bit 15.*/
void add_hl_r16(Gameboy *gb, r16_e idx){
    uint16_t value_r16 = read_r16(gb, idx);
    uint16_t value_hl = read_r16(gb, R16_HL);
    uint32_t result = value_r16 + value_hl; // Capturing overflow on bit 15

    gb->cpu.F &= Z_F; // clears the rest, so N=0

    if (((value_hl & 0x0FFF)+(value_r16 & 0x0FFF)) > 0x0FFF)
        gb->cpu.F |= H_F;
    if (result > 0xFFFF)
        gb->cpu.F |= C_F;

    write_r16(gb, R16_HL, (uint16_t)result);
}
/* Add the signed value e8 to SP
 * Z 0
 * N 0
 * H Set if overflow from bit 3.
 * C Set if overflow from bit 7. */
void add_sp_e8(Gameboy *gb){
    int8_t value_e8 = (int8_t)read_imm8(gb);
    uint16_t value_sp = read_r16(gb, R16_SP);
    uint16_t result = value_e8 + value_sp;

    gb->cpu.F = 0;

    if (((value_sp & 0x0F) + ((uint8_t)value_e8 & 0x0F)) > 0x0F)
        gb->cpu.F |= H_F;
    if (((value_sp & 0xFF) + ((uint8_t)value_e8 & 0xFF)) > 0xFF)
        gb->cpu.F |= C_F;

    write_r16(gb, R16_SP, result);
}
/* Add a value to A - checking flags and carry
 * Z Set if result is 0.
 * N 0
 * H Set if overflow from bit 3.
 * C Set if overflow from bit 7. */
static void add_a_val(Gameboy *gb, uint8_t value, bool with_carry){
    uint8_t a = read_r8(gb, R8_A);
    uint8_t carry = with_carry && (gb->cpu.F & C_F);
    uint16_t result = a + value + carry;

    gb->cpu.F = 0;
    if ((uint8_t)result == 0) gb->cpu.F |= Z_F;
    if (((a & 0x0F) + (value & 0x0F) + carry ) > 0x0F) gb->cpu.F |= H_F;
    if (result > 0xFF) gb->cpu.F |= C_F;

    write_r8(gb, R8_A, (uint8_t)result);
}
/* Subtract a value, checking flags
 * Z Set if result is 0.
 * N 1
 * H Set if borrow from bit 4.
 * C Set if borrow (i.e. if r8 > A). */
static void sub_a_val(Gameboy *gb, uint8_t value, bool with_carry){
    uint8_t a = read_r8(gb, R8_A);
    uint8_t carry = with_carry && (gb->cpu.F & C_F);
    uint16_t sub = value + carry;
    uint8_t result = a - sub;

    gb->cpu.F = N_F;

    if (result == 0) gb->cpu.F |= Z_F;
    if ((a & 0x0F) < ((value & 0x0F) + carry)) gb->cpu.F |= H_F;
    if (a < sub) gb->cpu.F |= C_F;

    write_r8(gb, R8_A, result);
}

void add_a_imm8(Gameboy *gb){
    add_a_val(gb, read_imm8(gb), false);
}
void adc_a_imm8(Gameboy *gb){
    add_a_val(gb, read_imm8(gb), true);
}
void sub_a_imm8(Gameboy *gb){
    sub_a_val(gb, read_imm8(gb), false);
}
void sbc_a_imm8(Gameboy *gb){
    sub_a_val(gb, read_imm8(gb), true);
}
/* Set A to the bitwise AND between the value in r8 and A.
 * Z Set if result is 0.
 * N 0
 * H 1
 * C 0 */
static void and_a_val(Gameboy *gb, uint8_t value){
    gb->cpu.F = H_F;
    uint8_t a = read_r8(gb, R8_A);
    uint8_t res = a & value;

    if (res == 0) gb->cpu.F |= Z_F;

    write_r8(gb, R8_A, res);
}
/* Set A to the bitwise XOR between the value in r8 and A.
 * Z Set if result is 0.
 * N 0
 * H 0
 * C 0 */
static void xor_a_val(Gameboy *gb, uint8_t value){
    gb->cpu.F = 0;
    uint8_t a = read_r8(gb, R8_A);
    uint8_t res = a ^ value;

    if (res == 0) gb->cpu.F |= Z_F;

    write_r8(gb, R8_A, res);
}
/* Set A to the bitwise OR between the value in r8 and A.
 * Z Set if result is 0.
 * N 0
 * H 0
 * C 0 */
static void or_a_val(Gameboy *gb, uint8_t value){
    gb->cpu.F = 0;
    uint8_t a = read_r8(gb, R8_A);
    uint8_t res = a | value;

    if (res == 0) gb->cpu.F |= Z_F;

    write_r8(gb, R8_A, res);
}
/* --- Normal ALU operations here --- */
void add_a(Gameboy *gb, r8_e idx){
    add_a_val(gb, read_r8(gb, idx), false);
}
void adc_a(Gameboy *gb, r8_e idx){
    add_a_val(gb, read_r8(gb, idx), true);
}
void sub_a(Gameboy *gb, r8_e idx){
    sub_a_val(gb, read_r8(gb, idx), false);
}
void sbc_a(Gameboy *gb, r8_e idx){
    sub_a_val(gb, read_r8(gb, idx), true);
}
void and_a(Gameboy *gb, r8_e idx){
    and_a_val(gb, read_r8(gb, idx));
}
void and_a_imm8(Gameboy *gb){
    and_a_val(gb, read_imm8(gb));
}
void xor_a(Gameboy *gb, r8_e idx){
    xor_a_val(gb, read_r8(gb, idx));
}
void xor_a_imm8(Gameboy *gb){
    xor_a_val(gb, read_imm8(gb));
}
void or_a(Gameboy *gb, r8_e idx){
    or_a_val(gb, read_r8(gb, idx));
}
void or_a_imm8(Gameboy *gb){
    or_a_val(gb, read_imm8(gb));
}
/* ComPare the value in A with the value in r8. This subtracts the value in r8
 * from A and sets flags accordingly, but discards the result.
 * Z Set if result is 0.
 * N 1
 * H Set if borrow from bit 4.
 * C Set if borrow (i.e. if r8 > A). */
static void cp_a_val(Gameboy *gb, uint8_t value){
    gb->cpu.F = N_F;
    uint8_t a = read_r8(gb, R8_A);
    uint8_t res = a - value;

    if (res == 0) gb->cpu.F |= Z_F;
    if ((a & 0x0F) < (value & 0x0F)) gb->cpu.F |= H_F;
    if (a < value) gb->cpu.F |= C_F;

    printf("CP A=%02x value=%02x res=%02x F=%02x\n", a, value, res, gb->cpu.F);
}
void cp_a(Gameboy *gb, r8_e idx){
    cp_a_val(gb, read_r8(gb, idx));
}
void cp_a_imm8(Gameboy *gb){
    cp_a_val(gb, read_imm8(gb));
}

/* ---- */

void nop(void){ /* Silly function to implement */ }
void stop(Gameboy *gb) { (void)read_imm8(gb); } // But ignores the result
/*
Enter CPU low-power consumption mode until an interrupt occurs.

The exact behavior of this instruction depends on the state of the IME flag,
and whether interrupts are pending (i.e. whether ‘[IE] & [IF]’ is non-zero):

If the IME flag is set: The CPU enters low-power mode until after an interrupt
is about to be serviced. The handler is executed normally, and the CPU resumes
execution after the HALT when that returns. If the IME flag is not set, and no
interrupts are pending: As soon as an interrupt becomes pending, the CPU
resumes execution. This is like the above, except that the handler is not
called. If the IME flag is not set, and some interrupt is pending: The CPU
continues execution after the HALT, but the byte after it is read twice in a
row (PC is not incremented, due to a hardware bug).
*/
void halt(Gameboy *gb) { gb->cpu.halted=true; }
/* DI
 * Disable Interrupts by clearing the IME flag.
 */
void di(Gameboy *gb){
    gb->cpu.IME = false;
    gb->cpu.ime_enable_pending = false;
}
/* EI
 * Enable Interrupts by setting the IME flag.
 * The flag is only set after the instruction following EI. */
void ei(Gameboy *gb) { gb->cpu.ime_enable_pending = true; }

/* Jumping to a absolute n16 address */
void jmp_a16(Gameboy *gb ) {
    gb->cpu.PC = read_imm16(gb);
}
/* Jumping to absolute n16 address if cc is true */
bool jp_cc_a16(Gameboy *gb, cc_e cc){
    bool cc_b = cc_true(gb, cc);
    uint16_t address = read_imm16(gb); // Have to consume the byte even if false

    if (cc_b)
        gb->cpu.PC = address;

    return cc_b;
}
/* The address jp to is the address following the opcode plus an 8bit offset */
void jr_e8(Gameboy *gb){
    int8_t rel_address = (int8_t)read_imm8(gb);
    gb->cpu.PC += rel_address;
}
/* Same as above, but only on condition cc*/
bool jr_cc_e8(Gameboy *gb, cc_e cc){
    int8_t rel_address = (int8_t)read_imm8(gb);
    bool cc_b = cc_true(gb, cc);

    if (cc_b)
        gb->cpu.PC += rel_address;

    printf("JR cc=%d rel=%d F=%02x taken=%d\n", cc, rel_address, gb->cpu.F, cc_b);

    return cc_b;
}
/*Jump to address in HL; effectively, copy the value in register HL into PC.*/
void jp_hl(Gameboy *gb){
    gb->cpu.PC = read_r16(gb, R16_HL);
}

/* Last helpers for block_0, rotate operations*/
/* rotate register A left, setting C_F accordingly, other flags 0 */
void rlca(Gameboy *gb){
    uint8_t r8_a = read_r8(gb, R8_A);
    uint8_t c_bit = (r8_a & 0x80) >> 7;

    r8_a <<= 1;
    r8_a |= c_bit;
    gb->cpu.F = c_bit ? C_F : 0;

    write_r8(gb, R8_A, r8_a);
}
/* rotate register a right, flags accordingly */
void rrca(Gameboy *gb){
    uint8_t r8_a = read_r8(gb, R8_A);
    uint8_t c_bit = r8_a & 1;

    r8_a >>= 1;
    r8_a |= (c_bit << 7);
    gb->cpu.F = c_bit ? C_F : 0;

    write_r8(gb, R8_A, r8_a);
}
/* rotates A left through the carry flag, so if top bit is high, and C flag was
 * 0, C flag is now one, and a 0 comes in */
void rla(Gameboy *gb){
    uint8_t r8_a = read_r8(gb, R8_A);
    uint8_t c_bit = (r8_a & 0x80) >> 7;

    r8_a <<= 1;
    r8_a |= (gb->cpu.F & C_F) ? 1 : 0 ;
    gb->cpu.F = c_bit ? C_F : 0;

    write_r8(gb, R8_A, r8_a);
}
/* Rotate register A right, through the carry flag. */
void rra(Gameboy *gb){
    uint8_t r8_a = read_r8(gb, R8_A);
    uint8_t c_bit = r8_a & 1;

    r8_a >>= 1;
    r8_a |= (gb->cpu.F & C_F) ? 0x80 : 0;
    gb->cpu.F = c_bit ? C_F : 0;

    write_r8(gb, R8_A, r8_a);
}
void daa(Gameboy *gb){}
void cpl(Gameboy *gb){}
void scf(Gameboy *gb){}
void ccf(Gameboy *gb){}


// ================ MAIN DECODERS =====================
/* this block is kind of messy */
int decode_block0(Gameboy *gb, uint8_t opcode)
{
    uint8_t y = (opcode >> 3) & 7;
    uint8_t z = opcode & 7;
    uint8_t p = y >> 1;
    uint8_t q = y & 1;

    switch (z) {
    case 0:
        switch (y) {
        case 0: printf("nop\n");                    nop();       return 4;
        case 1: printf("ld [a16], sp\n");   ld_a16_sp(gb);       return 20;
        case 2: printf("stop\n");                stop(gb);       return 4;
        case 3: printf("jr e8\n");              jr_e8(gb);       return 12;
        case 4: printf("jr NZ e8\n"); return jr_cc_e8(gb, CC_NZ) ? 12 : 8;
        case 5: printf("jr Z, e8\n"); return jr_cc_e8(gb, CC_Z)  ? 12 : 8;
        case 6: printf("jr NC, e8\n");return jr_cc_e8(gb, CC_NC) ? 12 : 8;
        case 7: printf("jr C, e8\n"); return jr_cc_e8(gb, CC_C)  ? 12 : 8;
        } break;
    case 1:
        switch (q) {
        case 0: ld_r16_imm16(gb, p); printf("ld n16\n");        return 12;
        case 1: add_hl_r16(gb, p); printf("add r16 to HL\n");   return 8;
        } break;
    case 2:
        switch (q) {
        case 0: ld_mem_r16_a(gb, p); return 8;
        case 1: ld_a_mem_r16(gb, p); return 8;
        } break;
    case 3:
        switch (q) {
        case 0: inc_r16(gb, p); printf("INC r16\n");       return 8;
        case 1: dec_r16(gb, p); printf("DEC r16\n");       return 8;
        }break;
    case 4: inc_r8(gb, y); printf("inc r8\n"); return (y==R8_HL)?12:4;
    case 5: dec_r8(gb, y);                     return (y==R8_HL)?12:4;
    case 6: ld_r8_imm8(gb, y); printf("ld r8 imm8\n");return (y==R8_HL)?12:8;
    case 7:
        switch (y){
        case 0: rlca(gb); return 4;
        case 1: rrca(gb); return 4;
        case 2: rla(gb); return 4;
        case 3: rra(gb); return 4;
        case 4:;
        case 5:;
        case 6:;
        case 7:;
        }printf("[ 0x%.2x ] ! opcode not implemented\n", opcode); return -1;
    }
    return 0;
}

/* With the helpers and logic broken out, this function become really nice!
 * Block 1 done!
 */
int decode_block1(Gameboy *gb, uint8_t opcode)
{
    if (opcode == HLT) {
        gb->cpu.halted = true;
        return 4;
    }

    r8_e dst = (opcode >> 3) & 7; // Clear, but maybe keep them as y and z?
    r8_e src = opcode & 7;

    ld_r8_r8(gb, dst, src);
    printf("Decode Block 1\n");
    return (dst == R8_HL || src == R8_HL) ? 8 : 4;
}
/* Same with block 2, we switch on op and src only */
int decode_block2(Gameboy *gb, uint8_t opcode)
{
    uint8_t op  = (opcode >> 3) & 7;  // which operation
    r8_e src = opcode & 7;            // which register

    switch (op) {
    case 0: printf("add src=%u\n", src); add_a(gb, src); break;
    case 1: printf("adc src=%u\n", src); adc_a(gb, src); break;
    case 2: printf("sub src=%u\n", src); sub_a(gb, src); break;
    case 3: printf("sbc src=%u\n", src); sbc_a(gb, src); break;
    case 4: printf("and src=%u\n", src); and_a(gb, src); break;
    case 5: printf("xor src=%u\n", src); xor_a(gb, src); break;
    case 6: printf("or src=%u\n", src);  or_a (gb, src); break;
    case 7: printf("cp src=%u\n", src);  cp_a (gb, src); break;
    }
    return (src == R8_HL) ? 8 : 4;
}

/* The last block has the extended block, and also looks like the place the rest
 * of the opcodes just got dumped. This is messy - I will start with the ones
 * encountered, c3 etc, and then eventually fill out the rest
 */
int decode_block3(Gameboy *gb, uint8_t opcode)
{
    uint8_t z = opcode & 7;
    uint8_t y = (opcode >> 3) & 7;
    uint8_t q = y & 1;
    uint8_t p = (y >> 1);

    switch (z) {
        case 0:
            switch (y){
            case 0: break;
            case 1: break;
            case 2: break;
            case 3: break;
            case 4: printf("ldh [a8], A\n"); ldh_mem_a8_a(gb); return 12;
            case 5: printf("add sp, e8\n"); add_sp_e8(gb); return 16;
            case 6: printf("ldh A, [a8]\n"); ldh_a_mem_a8(gb); return 12;
            case 7: printf("ld hl, sp+e8\n"); ld_hl_sp_e8(gb); return 12;
            }break;
        case 1:
            if (!q){
                printf("pop r16:%d\n",p); write_r16stk(gb, p, pop16(gb)); return 12;
            } else {
                switch (p){
                case 0: break;
                case 1: break;
                case 2: break;
                case 3: break;
                }
            } break;
        case 2:
            switch (y){
            case 0:
            case 1:
            case 2:
            case 3: printf("jp cc a16\n"); return jp_cc_a16(gb,(cc_e)y) ? 16:12;
            case 4: printf("ldh [C], A"); ldh_c_a(gb); return 8;
            case 5: printf("ld [a16], A\n"); ld_a16_a(gb); return 16;
            case 6: printf("ldh A, [C]\n"); ldh_a_c(gb); return 8;
            case 7: printf("ld a, [a16]\n"); ld_a_a16(gb); return 16;
            }break;
        case 3:
            switch (y){
            case 0: printf("jp to n16\n"); jmp_a16(gb); return 16;
            case 1: printf("[TODO] PREFIX\n"); return 0;
            case 2:
            case 3:
            case 4:
            case 5: for(;;); // HARDLOCK
            case 6: di(gb); printf("di\n"); return 4;
            case 7: ei(gb); printf("ei\n"); return 4;
            } break;
        case 4:
            switch (y){
            case 0: break;
            case 1: break;
            case 2: break;
            case 3: break;
            case 4:
            case 5:
            case 6:
            case 7: printf("HARDLOCK\n");for(;;); // HARDLOCK
            }break;
        case 5:
            if (!q){
                printf("push r16:%d\n",p); push16(gb, read_r16stk(gb, p)); return 16;
            } else {
                switch (p){
                case 0: break;
                case 1: break;
                case 2: break;
                case 3: break;
                }
            } break;
        case 6:
            switch (y){
            case 0: printf("add imm8 a\n"), add_a_imm8(gb); return 8;
            case 1: printf("adc imm8 a\n"), adc_a_imm8(gb); return 8;
            case 2: printf("sub imm8 a\n"), sub_a_imm8(gb); return 8;
            case 3: printf("sbc imm8 a\n"), sbc_a_imm8(gb); return 8;
            case 4: printf("and imm8 a\n"), and_a_imm8(gb); return 8;
            case 5: printf("xor imm8 a\n"), xor_a_imm8(gb); return 8;
            case 6: printf("or imm8 a\n"), or_a_imm8(gb); return 8;
            case 7: printf("cp imm8 a\n"), cp_a_imm8(gb); return 8;
            }break;
        case 7:
            switch (y){
            case 0: break;
            case 1: break;
            case 2: break;
            case 3: break;
            case 4: break;
            case 5: break;
            case 6: break;
            case 7: break;
            }break;
    }

    printf("3 [ 0x%.2x ] ! opcode not implemented\n", opcode); return -1;
}
