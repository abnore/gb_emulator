#include <blackbox.h>
#include "decoder.h"
#include "bus.h"
#include "cpu.h"



/* Main decoder function
 *
 * We will switch om the opcode block - either 0,1,2 or 3.
 * 0 contains specials, and sub groups divided by the 4th bit, q, and enumerated
 * on p. 4 in each sub groups. Block 1 is the LD block, r8,r8. block 2 is ALU
 * and block 3 is stack, vectors, and misc
 * */
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
            case 0: TRACE("NOP");           break;
            case 1: TRACE("LD [a16], SP");  break;
            case 2: TRACE("STOP");          break;
            case 3: TRACE("JR e8");         break;
            case 4: TRACE("JR NZ, e8");     break;
            case 5: TRACE("JR Z, e8");      break;
            case 6: TRACE("JR NC, e8");     break;
            case 7: TRACE("JR C, e8");      break;
            }
            break;
        case 1: // z=1 load/add n16 based on q and p
            switch (q) {
            case 0: TRACE("LD something");  break;
            case 1: TRACE("ADD something"); break;
            }
            break;
        case 2: // z=2 load to/from A based on q and p
            switch (q) {
            case 0: TRACE("LD from A");     break;
            case 1: TRACE("LD to A");       break;
            }break;
        case 3: // z=3 inc or dec r16 based on q and p
            switch (q) {
            case 0: TRACE("INC r16");       break;
            case 1: TRACE("DEC r16");       break;
            }break;
        case 4: // z=4 inc r8 based on y
            TRACE("INC r8");                break;
        case 5: // z=5 dec r8 based on y
            TRACE("DEC r8");                break;
        case 6: // z=6 LD n8 into r8 based on y
            TRACE("LD something n8");       break;
        case 7: // rotate flag based on y
            TRACE("Rotate");                break;
        default:
            ASSERT(0, "unreachable");
    };
}
void decode_block1(CPU *cpu, Bus *bus, uint8_t opcode){

}
void decode_block2(CPU *cpu, Bus *bus, uint8_t opcode){
    if (opcode == 0xaf) {
        TRACE("XOR A,A");
        cpu->A = 0;
        cpu->F = Z_F;
    }
}
void decode_block3(CPU *cpu, Bus *bus, uint8_t opcode){
    if (opcode == 0xc3) {
        TRACE("JP to n16");
        byte lo = bus_read(bus, cpu->PC++);
        byte hi = bus_read(bus, cpu->PC++);
        cpu->PC = (hi << 8) | lo;
    }
}

