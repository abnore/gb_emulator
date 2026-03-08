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
    CPU cpu = {.IME=true, .AF = 0xaa00, .BC = 0xbbcc };

    load_cartridge(&rom, argv[1]);

    INFO("Loaded cartridge! We now have access to the data");

    printf("AF: 0x%.4x, in the A is 0x%.2x and F is 0x%.2x\n", cpu.AF, cpu.A, cpu.F);
    printf("BC: 0x%.4x, in the B is 0x%.2x and C is 0x%.2x\n", cpu.BC, cpu.B, cpu.C);

    /* $0100—$014F */

    for(int i = 0x100; i < 0x15f; i+=0x10){
        printf("0x%.4x  ", i);

        for(int j=i; j<i+0x10; ++j){
            printf("0x%.2x ",(uint8_t)rom.data[j]);
        }
        printf("\n");
    }
    /* So now we can read something, and extract register states. Lets see what
     * else we can do */

    set_flag(Z_F, &cpu.F);
    printf("flags: 0x%.8x\n", cpu.F);
    set_flag(S_F, &cpu.F);
    printf("flags: 0x%.8x\n", cpu.F);
    printf("Is flag set: %x\n", test_flag(Z_F, cpu.F));
    unset_flag(Z_F, &cpu.F);
    printf("flags: 0x%.8x\n", cpu.F);

    printf("Is flag set: %x\n", test_flag(Z_F, cpu.F));


    while (cpu.IME){
        printf("ok\n");
    }
    remove_cartridge(&rom);
    shutdown_log();
    return 0;
}
