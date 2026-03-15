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
 * OK read_imm8
 * OK read_imm16
 *
 * OK read_r8
 * OK write_r8
 * OK read_r16
 * OK write_r16
 * OK read_r16stk
 * OK write_r16stk
 *
 * OK push16
 * OK pop16
 * OK cc_true
 *
 * OK ld_r8_r8
 * OK ld_r8_imm8
 * OK ld_r16_imm16
 * OK ld_mem_hl_imm8
 * OK addr_r16mem
 * OK step_r16mem
 * OK ld_mem_r16_a
 * OK ld_a_mem_r16
 * OK ld_a16_sp
 * OK ld_a16_a
 * OK ld_a_a16
 * ld_sp_hl
 * ld_hl_sp_e8
 *
 * inc_r8
 * dec_r8
 * inc_r16
 * dec_r16
 * add_hl_r16
 * add_sp_e8
 *
 * add_a
 * adc_a
 * sub_a
 * sbc_a
 * and_a
 * xor_a
 * or_a
 * cp_a
 *
 * rlca
 * rrca
 * rla
 * rra
 * daa
 * cpl
 * scf
 * ccf
 *
 * jr_e8
 * jr_cc_e8
 * jp_a16
 * jp_cc_a16
 * jp_hl
 * call_a16
 * call_cc_a16
 * ret
 * ret_cc
 * reti
 * rst
 *
 * halt
 * stop
 * di
 * ei
 *
 * cb_dispatch
 * rot_r8
 * bit_r8
 * res_r8
 * set_r8


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
static uint8_t read_imm8(Gameboy *gb){
    return bus_read(gb, gb->cpu.PC++);
}
/* Reading two bytes, little endian, and return a 16bit value */
static uint16_t read_imm16(Gameboy *gb){
    uint8_t lo = read_imm8(gb);
    uint8_t hi = read_imm8(gb);
    return ((uint16_t)hi << 8) | lo;
}
/* Read from a 8bit register based on index: B,C,D,E,H,L,[HL],A,
 * notigce [HL] is the byte at the address in HL */
static uint8_t read_r8(Gameboy *gb, r8_e idx){
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
static void write_r8(Gameboy *gb, r8_e idx, uint8_t value){
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
static uint16_t read_r16(Gameboy *gb, r16_e idx){
    switch (idx) {
    case R16_BC:  return gb->cpu.BC;
    case R16_DE:  return gb->cpu.DE;
    case R16_HL:  return gb->cpu.HL;
    case R16_SP:  return gb->cpu.SP;
    }
    return 0xffff;
}
/* Same as above, only writing */
static void write_r16(Gameboy *gb, r16_e idx, uint16_t value){
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

static uint16_t read_r16stk(Gameboy *gb, r16stk_e idx){
    switch (idx) {
    case R16STK_BC:  return gb->cpu.BC;
    case R16STK_DE:  return gb->cpu.DE;
    case R16STK_HL:  return gb->cpu.HL;
    case R16STK_AF:  return gb->cpu.AF;
    }
    return 0xffff;
}
/* Same only writing */
static void write_r16stk(Gameboy *gb, r16stk_e idx, uint16_t value){
    switch (idx) {
    case R16STK_BC: gb->cpu.BC = value; break;
    case R16STK_DE: gb->cpu.DE = value; break;
    case R16STK_HL: gb->cpu.HL = value; break;
    case R16STK_AF: gb->cpu.AF = value; break;
    }
}

/* Returns the address pointed to by [BC] and [DE], also
 * [HL] with an increment and decrement */
static uint16_t addr_r16mem(Gameboy *gb, r16mem_e op){
    switch (op) {
    case R16MEM_BC:     return gb->cpu.BC;
    case R16MEM_DE:     return gb->cpu.DE;
    case R16MEM_HL_INC: return gb->cpu.HL;
    case R16MEM_HL_DEC: return gb->cpu.HL;
    }
    return 0xffff;
}
static void step_r16mem(Gameboy *gb, r16mem_e op){
    switch (op) {
    case R16MEM_HL_INC: gb->cpu.HL++; break;
    case R16MEM_HL_DEC: gb->cpu.HL--; break;
    default: break; // Not stepping BC and DE
    }
}
/* Pushes a 16bit value on the stack, first decrements SP and then write high
 * byte first */
static void push16(Gameboy *gb, uint16_t value) {
    bus_write(gb, --(gb->cpu.SP), (uint8_t)(value >> 8));
    bus_write(gb, --(gb->cpu.SP), (uint8_t)value);
}
/* Pops a 16bit value from the stack, and updating the SP */
static uint16_t pop16(Gameboy *gb) {
    uint8_t lo = bus_read(gb, gb->cpu.SP++);
    uint8_t hi = bus_read(gb, gb->cpu.SP++);
    return ((uint16_t)hi << 8) | lo;
}
static int cc_true(Gameboy *gb, cc_e cc){
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
static void ld_r8_r8(Gameboy *gb, r8_e dst, r8_e src){
    write_r8(gb, dst, read_r8(gb, src));
}
static void ld_r8_imm8(Gameboy *gb, r8_e dst){
    write_r8(gb, dst, read_imm8(gb));
}
static void ld_r16_imm16(Gameboy *gb, r16_e dst){
    write_r16(gb, dst, read_imm16(gb));
}
static void ld_mem_hl_imm8(Gameboy *gb){
    write_r8(gb, R8_HL, read_imm8(gb));
}

/* Uses [BC], [DE] and [HL+] [HL-] */
static void ld_mem_r16mem_a(Gameboy *gb, r16mem_e dst){
    bus_write(gb, addr_r16mem(gb, dst), gb->cpu.A);
    step_r16mem(gb, dst);
}

static void ld_a_mem_r16mem(Gameboy *gb, r16mem_e src){
    write_r8(gb, R8_A, bus_read(gb, addr_r16mem(gb, src)));
    step_r16mem(gb, src);
}

/* LD  using an imm16bit address operand, a16, first two are a store X a16 */
/* First the LD [a16], SP. We need to interprete the a16 as an address, then
 * write the contents of SP to that address, in little endian, since SP is 16bit
 * we write 2 bytes, starting from the address */
static void ld_a16_sp(Gameboy *gb){
    uint16_t addr = read_imm16(gb);
    bus_write(gb, addr,     (uint8_t)gb->cpu.SP);
    bus_write(gb, addr + 1, (uint8_t)(gb->cpu.SP >> 8));
}

/* This does the same, only now from A, we store only 1 byte
 * LD [a16], A */
static void ld_a16_a(Gameboy *gb){
    uint16_t addr = read_imm16(gb);
    bus_write(gb, addr, gb->cpu.A);
}

/* LD A, [a16] */
static void ld_a_a16(Gameboy *gb){
    uint16_t addr = read_imm16(gb);
    write_r8(gb, R8_A, bus_read(gb, addr));
}

/* LD SP, HL and the special LD HL, SP+e8 form */
static void ld_sp_hl(Gameboy *gb){
    write_r16(gb, R16_SP, read_r16(gb, R16_HL));
}
/* Add the signed value e8 to SP and copy the result in HL. This instruction
 * sets flags: Z and N is set to 0, and H Set if overflow from bit 3. C Set if
 * overflow from bit 7. */
static void ld_hl_sp_e8(Gameboy *gb){
    uint16_t sp = read_r16(gb, R16_SP);
    int8_t e8 = (int8_t)read_imm8(gb);
    uint16_t result = sp + e8;
    gb->cpu.F = 0;

    if (((sp & 0x0F) + ((uint8_t)e8 & 0x0F)) > 0x0F) gb->cpu.F |= H_F;
    if (((sp & 0xFF) + ((uint8_t)e8 & 0xFF)) > 0xFF) gb->cpu.F |= C_F;

    write_r16(gb, R16_HL, result);
}

/* Jumping to a absolute n16 address */
static void jmp(Gameboy *gb )
{
    gb->cpu.PC = read_imm16(gb);
}


static void cmp(Gameboy *gb, int something_else){
    (void)gb, (void)something_else;
}

int decode_block0(Gameboy *gb, uint8_t opcode)
{
    int cycles=0;
    uint8_t y = (opcode >> 3) & 7;
    uint8_t z = opcode & 7;
    //uint8_t p = y >> 1; Maybe this isnt used for anything
    uint8_t q = y & 1;
    switch (z) {
        case 0: // z=0 specials
            switch (y) {
            case 0: printf("NOP\n"); cycles = 4;                        break;
            case 1: printf("LD [a16], SP\n"); cycles=20;                break;
            case 2: printf("STOP\n"); gb->cpu.PC++; gb->cpu.halted=true;  break;
            case 3: printf("JR e8\n");         break;
            case 4: printf("JR NZ, e8\n");     break;
            case 5: printf("JR Z, e8\n");      break;
            case 6: printf("JR NC, e8\n");     break;
            case 7: printf("JR C, e8\n");      break;
            }
            break;
        case 1: // z=1 load/add n16 based on q and p
            switch (q) {
            case 0: printf("LD n16 based on p and q\n");  break;
            case 1: printf("ADD r16 to HL\n"); break;
            }
            break;
        case 2: // z=2 load to/from A based on q and p
            switch (q) {
            case 0: printf("LD from A to address of r16\n");     break;
            case 1: printf("LD to A from address of r16\n");     break;
            }break;
        case 3: // z=3 inc or dec r16 based on q and p
            switch (q) {
            case 0: printf("INC r16\n");       break;
            case 1: printf("DEC r16\n");       break;
            }break;
        case 4: // z=4 inc r8 based on y
            printf("INC r8\n");                break;
        case 5: // z=5 dec r8 based on y
            printf("DEC r8\n");                break;
        case 6: // z=6 LD n8 into r8 based on y
            printf("LD something n8\n");
            gb->cpu.PC++; break;
        case 7: // rotate flag based on y
            printf("Rotate + div...\n");                break;
    };

    return cycles;
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

    r8_e dst = (opcode >> 3) & 7;
    r8_e src = opcode & 7;

    ld_r8_r8(gb, dst, src);
    printf("Decode Block 1\n");
    return (dst == R8_HL || src == R8_HL) ? 8 : 4;
}

/* ALU will do ADD, ADC, SUB, SBC, AND, XOR, OR and CP on different registers*/
static void ALU(Gameboy *gb, int op, uint16_t reg, char *str){

    switch (op) {
    case 0:printf("ADD A, %s\n", str); gb->cpu.A += reg; break;
    case 1:printf("ADC A, %s\n", str); gb->cpu.A += reg; gb->cpu.F |= C_F; break;
    case 2:printf("SUB A, %s\n", str); gb->cpu.A -= reg; break;
    case 3:printf("SBC A, %s\n", str); gb->cpu.A -= reg; gb->cpu.F |= C_F; break;
    case 4:printf("AND A, %s\n", str); gb->cpu.A &= reg; break;
    case 5:printf("XOR A, %s\n", str); gb->cpu.A ^= reg; break;
    case 6:printf("OR A, %s\n", str); gb->cpu.A |= reg; break;
    case 7:printf("CP A ,%s \n", str); cmp(gb, 0); break;
    }
}

int decode_block2(Gameboy *gb, uint8_t opcode)
{
    int cycles=0;
    uint8_t y = (opcode >> 3) & 7;  // which operation
    uint8_t z = opcode & 7;         // which register

    switch (z) {
        case 0: ALU(gb, y, gb->cpu.B, "B"); break;
        case 1: ALU(gb, y, gb->cpu.C, "C"); break;
        case 2: ALU(gb, y, gb->cpu.D, "D"); break;
        case 3: ALU(gb, y, gb->cpu.E, "E"); break;
        case 4: ALU(gb, y, gb->cpu.H, "H"); break;
        case 5: ALU(gb, y, gb->cpu.L, "L"); break;
        case 6: ALU(gb, y, bus_read(gb, gb->cpu.HL), "[HL]"); break;
        case 7: ALU(gb, y, gb->cpu.A, "A"); break;
    }

    return cycles;
}

/* The last block has the extended block, and also looks like the place the rest
 * of the opcodes just got dumped. This is messy - I will start with the ones
 * encountered, c3 etc, and then eventually fill out the rest
 */
int decode_block3(Gameboy *gb, uint8_t opcode)
{
    int cycles=0;
    uint8_t z = opcode & 7;
    uint8_t y = (opcode >> 3) & 7;

    switch (z) {
        case 0:
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
        case 1:
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
        case 2:
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
        case 3:
            switch (y){
            case 0: printf("JP to n16\n"); jmp(gb); return 0;
            case 1: printf("PREFIX\n"); return 0;
            case 2:;
            case 3:;
            case 4:;
            case 5:for(;;); // HARDLOCK
            case 6: printf("DI\n"); return 0;
            case 7: printf("EI\n"); return 0;
            } break;
        case 4:
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
        case 5:
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
        case 6:
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

    printf("[ 0x%.2x ] ! opcode not implemented\n", opcode);
    return cycles;
}
