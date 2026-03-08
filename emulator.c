/* Gameboy emulator
 *
 * author: Andreas Nore - github.com/abnore
 */
#include <blackbox.h>

#include "rom.h"
#include "cpu.h"

int main(int argc, char **argv)
{
    init_log(LOG_DEFAULT);

    switch (argc){
        case 1:
            FATAL("Needs a rom file to run\nUsage: ... TODO");
            return 1;
        case 2: // correct
            break;
        case 3:
        default:
            FATAL("Only one (1) file accepted, no flags");
            return 2;
    }
    /* --- Here we have the rom and is ready to go --- */
    ROM rom;
    CPU cpu = {.IME=true };

    load_cartridge(&rom, argv[1]);
    INFO("Loaded cartridge! We now have access to the data");

    /* $0100—$014F */
    // for(int i = 0x100; i < 0x15f; i+=0x10){
    //     printf("0x%.4x  ", i);
    //
    //     for(int j=i; j<i+0x10; ++j){
    //         printf("0x%.2x ",rom.data[j]);
    //     }
    //     printf("\n");
    // }

    /* So now we can read something, and extract register states. Lets see what
     * else we can do */

    cpu.PC=0x0100; // Initialized at the entry point

    for (int i=0; i<60; ++i){

        printf("Cpu cycles %.2i - PC: 0x%.4x ", i, cpu.PC);
        byte opcode = rom.data[cpu.PC++];

        switch (opcode){
            case 0x00: // NOP 4 cycles
                printf("0x%x: NOP\n", opcode);
                i+=4-1;
                break;

            case 0xc3: // JMP 8 cycles
                printf("0x%x: JMP, TO?.. ", opcode);
                byte lo = rom.data[cpu.PC++];
                byte hi = rom.data[cpu.PC++];
                uint16_t address = (hi<<8) | lo;
                printf("0x%.4x\n", address);
                cpu.PC = address;
                i+=8-1;
                break;

            case 0xaf: // XOR A,A 4 cycles
                printf("0x%x: XOR A,A ok, sure, zero out A\n", opcode);
                cpu.A = 0;
                i+=4-1;
                break;

            case 0x21: // LD HL 12 cycles
                printf("0x%x LD HL, what?\n", opcode);
                printf("Cpu cycles %.2i - PC: 0x%.4x ", i, cpu.PC);
                cpu.L = rom.data[cpu.PC++];
                printf("0x%.2x - low byte\n", cpu.L);
                cpu.H = rom.data[cpu.PC++];
                printf("Cpu cycles %.2i - PC: 0x%.4x ", i, cpu.PC);
                printf("0x%x - 16 bit value\n", cpu.HL);
                i+=12-1;
                break;

            case 0x0e: // LD C 8 cycles
                printf("0x%.2x LD C, what?\n", opcode);
                cpu.C = rom.data[cpu.PC++];
                printf("Cpu cycles %.2i - PC: 0x%.4x ", i, cpu.PC);
                printf("0x%.2x - value in C now\n", cpu.C);
                i+=8-1;
                break;

            case 0x06: // LD B 8 cycles
                printf("0x%.2x LD B, what?\n", opcode);
                cpu.B = rom.data[cpu.PC++];
                printf("Cpu cycles %.2i - PC: 0x%.4x ", i, cpu.PC);
                printf("0x%.2x - value in B now\n", cpu.B);
                i+=8-1;
                break;

            default:
                printf("0x%.2x\n", opcode);
        }
    }
    remove_cartridge(&rom);
    shutdown_log();
    return 0;
}
