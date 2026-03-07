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
            FATAL("Needs a file file to run\nUsage: ... TODO");
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
    CPU cpu = {.AF = 0xaa55, .BC = 0xbbcc };
    load_cartridge(&rom, argv[1]);

    INFO("Loaded cartridge! We now have access to the data");

    INFO("AF containing %x, in the A is %x and F is %x", cpu.AF, cpu.A, cpu.F);
    INFO("BC containing %x, in the B is %x and C is %x", cpu.BC, cpu.B, cpu.C);

    /* $0100—$014F */

    for(int i = 0x100; i < 0x15f; i+=0x10){
        printf("0x%.4x  ", i);

        for(int j=i; j<i+0x10; ++j){
            printf("0x%.2x ",(uint8_t)rom.data[j]);
        }
        printf("\n");
    }
    printf("\n");
    remove_cartridge(&rom);
    shutdown_log();
    return 0;
}
