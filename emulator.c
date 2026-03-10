/* Gameboy emulator
 *
 * author: Andreas Nore - github.com/abnore
 */
#include <blackbox.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

#include "rom.h"
#include "cpu.h"
#include "opcodes.h"
#include "clock.h"

int main(int argc, char **argv)
{
    init_log(LOG_DEFAULT);

    if (argc < 2){
        FATAL("Needs a rom file to run\nUsage: ... TODO");
        return 1;
    } else if (argc > 2){
        FATAL("Only one (1) file accepted, no flags");
        return 3;
    }
    if (access(argv[1], F_OK)){
        FATAL("%s doesn't exist",basename(argv[1]));
        return 2;
    }

    /* --- Here we have the rom and is ready to go --- */
    init_clock();

    CPU cpu = { .running=true, .IME = true, .PC = ENTRY_POINT, .SP = 0xdfff };

    off_t rom_size;
    ROM rom = load_cartridge(argv[1], &rom_size);


    /* Starting to check opcodes and investigating running the ROM */
    while( cpu.running ) {
        //if( next_cycle() ) {

            int i=0;
            printf("Cpu cycles %.2i - PC: 0x%.4x ", i, cpu.PC);
            byte opcode = rom[cpu.PC++];

            switch (opcode){
                case NOP: // NOP 4 cycles
                    printf("0x%.2x: NOP\n", opcode);
                    i+=4-1;
                    break;

                case JMP: // JMP 8 cycles
                    printf("0x%.2x: JMP, TO?.. ", opcode);
                    byte lo = rom[cpu.PC++];
                    byte hi = rom[cpu.PC++];
                    uint16_t address = (hi<<8) | lo;
                    printf("0x%.4x\n", address);
                    cpu.PC = address;
                    i+=8-1;
                    break;

                case XRA: // XOR A,A 4 cycles
                    printf("0x%.2x: XOR A,A ok, sure, zero out A\n", opcode);
                    cpu.A = 0;
                    i+=4-1;
                    break;

                case LHL: // LD HL 12 cycles
                    printf("0x%.2x: LD HL -> ", opcode);
                    cpu.L = rom[cpu.PC++];
                    printf("lowbyte: 0x%.2x ==> ", cpu.L);
                    cpu.H = rom[cpu.PC++];
                    printf("0x%x - 16 bit value\n", cpu.HL);
                    i+=12-1;
                    break;

                case LDC: // LD C 8 cycles
                    printf("0x%.2x: LD C -> ", opcode);
                    cpu.C = rom[cpu.PC++];
                    printf("0x%.2x\n", cpu.C);
                    i+=8-1;
                    break;

                case LDB: // LD B 8 cycles
                    printf("0x%.2x: LD B -> ", opcode);
                    cpu.B = rom[cpu.PC++];
                    printf("0x%.2x\n", cpu.B);
                    i+=8-1;
                    break;

                case LAH: //LD value of A in [HL] and decrement HL afterwards.
                    printf("0x%.2x: LD value of A into [HL]\n", opcode);
                    break;
                    i+=8-1;
                default:
                    cpu.running=false;
                    printf("0x%.2x: ? [ opcode not implemented ]\n", opcode);
            }
            i++;
        //}
    }
    remove_cartridge(rom, rom_size);
    shutdown_log();
    return 0;
}
