/* Gameboy emulator
 *
 * author: Andreas Nore - github.com/abnore
 */
#include <blackbox.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

#include "rom.h"
#include "bus.h"
#include "cpu.h"
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

    CPU cpu = { .IME = true, .PC = ENTRY_POINT, .SP = 0xdfff };
    Bus bus;

    bus.rom = load_cartridge(argv[1], &bus.rom_size);

    int safe_steps = 0;
    while ( safe_steps < 100/*!cpu.halted*/ ){
        cpu_step(&cpu, &bus);
        safe_steps++;
    }

    /* Starting to check opcodes and investigating running the ROM */
    remove_cartridge(bus.rom, bus.rom_size);
    shutdown_log();
    return 0;
}
