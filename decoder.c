/* Main decoder function
 *
 * We will switch on the opcode block - either 0,1,2 or 3 based on first 2 bits.
 *
 * The logic will be based on a fixed ordering on the registers internally.
 * When its r16 it is always BC, DE, HL, SP
 * when its r8 it is always B,C,D,E,H,L,[HL],A
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
 */

#include <blackbox.h>
#include "decoder.h"
#include "bus.h"
#include "cpu.h"

static uint8_t read_r8(CPU *cpu, Bus *bus){

}

static uint8_t write_r8(CPU *cpu, Bus *bus){

}
static uint16_t read_r16(CPU *cpu, Bus *bus){

}
static uint16_t write_r16(CPU *cpu, Bus *bus){

}

static void alu_op(void){

}

static void cc_true(void){

}
#define HLT 0x76
/* Jumping to a absolute n16 address */
static void jmp(CPU *cpu, Bus *bus )
{
    uint8_t lo = bus_read(bus, cpu->PC++);
    uint8_t hi = bus_read(bus, cpu->PC++);
    cpu->PC = (hi << 8) | lo;
}

/* Jumping to a relative offset of SP */
static void jmp_rel(CPU *cpu, Bus *bus){
    uint8_t offset = bus_read(bus, cpu->PC++);
    cpu->PC += offset;
}

/* LD wrapper for block 1 */
static void ld(CPU *cpu, Bus *bus, uint8_t y, uint8_t op2, char *str_op2)
{
    uint8_t *op1;
    char *str_op1="";
    switch (y) {
    case 0: op1 = &cpu->B; str_op1="B"; break;
    case 1: op1 = &cpu->D; str_op1="D";break;
    case 2: op1 = &cpu->H; str_op1="H";break;
    case 3: op1 = (uint8_t*)bus_read(bus,cpu->HL); str_op1="[HL]"; break;
    case 4: op1 = &cpu->C; str_op1="C";break;
    case 5: op1 = &cpu->E; str_op1="E"; break;
    case 6: op1 = &cpu->L; str_op1="L"; break;
    case 7: op1 = &cpu->A; str_op1="A"; break;
    }
    printf("LD %s - 0x%.2x into %s\n", str_op2, op2, str_op1);
     *op1 = op2;
}

static void cmp(CPU *cpu, Bus *bus, int something_else){
 (void)cpu, (void)bus, (void)something_else;
}
/* ALU will do ADD, ADC, SUB, SBC, AND, XOR, OR and CP on different registers*/
static void ALU(CPU *cpu, Bus *bus, int op, uint16_t reg, char *str){

    switch (op) {
    case 0:printf("ADD A, %s\n", str); cpu->A += reg; break;
    case 1:printf("ADC A, %s\n", str); cpu->A += reg; cpu->F |= C_F; break;
    case 2:printf("SUB A, %s\n", str); cpu->A -= reg; break;
    case 3:printf("SBC A, %s\n", str); cpu->A -= reg; cpu->F |= C_F; break;
    case 4:printf("AND A, %s\n", str); cpu->A &= reg; break;
    case 5:printf("XOR A, %s\n", str); cpu->A ^= reg; break;
    case 6:printf("OR A, %s\n", str); cpu->A |= reg; break;
    case 7:printf("CP A ,%s \n", str); cmp(cpu, bus, 0); break;
    }

}
void decoder(CPU *cpu, Bus *bus, uint8_t opcode){
    uint8_t block = opcode >> 6;

    switch (block) {
    case 0: decode_block0(cpu, bus, opcode); break;
    case 1: decode_block1(cpu, bus, opcode); break;
    case 2: decode_block2(cpu, bus, opcode); break;
    case 3: decode_block3(cpu, bus, opcode); break;
    }
}

void decode_block0(CPU *cpu, Bus *bus, uint8_t opcode)
{
    uint8_t y = (opcode >> 3) & 7;
    uint8_t z = opcode & 7;
    uint8_t p = y >> 1;
    uint8_t q = y & 1;

    switch (z) {
        case 0: // z=0 specials
            switch (y) {
            case 0: printf("NOP\n");           break;
            case 1: printf("LD [a16], SP\n");  break;
            case 2: printf("STOP\n"); cpu->halted=true;  break;
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
            cpu->PC++; break;
        case 7: // rotate flag based on y
            printf("Rotate + div...\n");                break;
        default:
            ASSERT(0, "unreachable\n");
    };
}
void decode_block1(CPU *cpu, Bus *bus, uint8_t opcode)
{
    if (opcode == HLT){
        cpu->halted=true;
        return;
    }
    int y = (opcode >> 3) & 7;
    int z = opcode & 7;

    uint8_t op2;
    char *str_op2="";
    switch (z) {
    case 0: op2 = cpu->B; str_op2="B"; break;
    case 1: op2 = cpu->C; str_op2="C";break;
    case 2: op2 = cpu->D; str_op2="D";break;
    case 3: op2 = cpu->E; str_op2="E";break;
    case 4: op2 = cpu->H; str_op2="H"; break;
    case 5: op2 = cpu->L; str_op2="L"; break;
    case 6: op2 = bus_read(bus,cpu->HL); str_op2="[HL]"; break;
    case 7: op2 = cpu->A; str_op2="A"; break;
    }
    ld(cpu, bus, y, op2, str_op2);
}
void decode_block2(CPU *cpu, Bus *bus, uint8_t opcode)
{
    uint8_t y = (opcode >> 3) & 7;  // which operation
    uint8_t z = opcode & 7;         // which register


    switch (z) {
        case 0: ALU(cpu, bus, y, cpu->B, "B"); break;
        case 1: ALU(cpu, bus, y, cpu->C, "C"); break;
        case 2: ALU(cpu, bus, y, cpu->D, "D"); break;
        case 3: ALU(cpu, bus, y, cpu->E, "E"); break;
        case 4: ALU(cpu, bus, y, cpu->H, "H"); break;
        case 5: ALU(cpu, bus, y, cpu->L, "L"); break;
        case 6: ALU(cpu, bus, y, bus_read(bus, cpu->HL), "[HL]"); break;
        case 7: ALU(cpu, bus, y, cpu->A, "A"); break;
    }
}

/* The last block has the extended block, and also looks like the place the rest
 * of the opcodes just got dumped. This is messy - I will start with the ones
 * encountered, c3 etc, and then eventually fill out the rest
 */
void decode_block3(CPU *cpu, Bus *bus, uint8_t opcode){
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
            case 0: printf("JP to n16\n"); jmp(cpu, bus); return;
            case 1: printf("PREFIX\n"); return;
            case 2:;
            case 3:;
            case 4:;
            case 5:for(;;); // HARDLOCK
            case 6: printf("DI\n"); return;
            case 7: printf("EI\n"); return;
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
}
