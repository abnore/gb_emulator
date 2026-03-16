#include "gameboy.h"
#include "decoder.h"
#include <blackbox.h>

/* I am getting sick of passing the cpu and bus to every function. There should
 * be a larger structure, or a global that holds both
 */
Gameboy gb_init(){
    return (Gameboy){
        .cpu.IME    = true,
        .cpu.PC     = ENTRY_POINT,
        .cpu.SP     = 0xdffe,
        .bus.rom    = NULL,
        .cycles     = 0,
    };
}

/* Memory Map
 * The Game Boy has a 16-bit address bus, which is used to address ROM, RAM,
 * and I/O.
 *
 * Start End Description
 * 0000 3FFF 16 KiB ROM bank 00	From cartridge, usually a fixed bank
 * 4000 7FFF 16 KiB ROM Bank 01–NN From cartridge, switchable bank via mapper
 * A000 BFFF 8 KiB External RAM	From cartridge, switchable bank if any
 * C000 CFFF 4 KiB Work RAM (WRAM)
 * D000 DFFF 4 KiB Work RAM (WRAM)	In CGB mode, switchable bank 1–7
 * FE00 FE9F Object attribute memory (OAM)
 * FF00 FF7F I/O Registers
 * FF80 FFFE High RAM (HRAM)
 * FFFF FFFF Interrupt Enable register (IE)
 * Abstracting away the bus so that the cpu do not have to thing about
 * ROM, RAM or I/0. The CPU will just write or read from an address, and
 * the this way it is memory mapped.
 *
 * This will work for now, only touching what i have implemented so far
 */
uint8_t bus_read(Gameboy *gb, uint16_t addr)
{
    Bus *bus = &gb->bus;

    if (addr <= 0x7FFF)
        return bus->rom[addr];

    if (addr >= 0xC000 && addr <= 0xDFFF)
        return bus->wram[addr - 0xC000];

    if (addr == 0xFF0F)
        return bus->i_flag;

    if (addr >= 0xFF80 && addr <= 0xFFFE)
        return bus->hram[addr - 0xFF80];

    if (addr == 0xFFFF)
        return bus->i_enable;

    if (addr == 0xFF44)
        return 0x94;

    return 0xFF;
}

void bus_write(Gameboy *gb, uint16_t addr, uint8_t value)
{
    Bus *bus = &gb->bus;

    if (addr >= 0xC000 && addr <= 0xDFFF) {
        bus->wram[addr - 0xC000] = value;
        return;
    }
    if (addr == 0xFF0F) {
        bus->i_flag = value;
        return;
    }
    if (addr >= 0xFF80 && addr <= 0xFFFE) {
        bus->hram[addr - 0xFF80] = value;
        return;
    }
    if (addr == 0xFFFF) {
        bus->i_enable = value;
        return;
    }
}

/* Fetched the opcode from the ROM and updated the program counter */
uint8_t fetch(Gameboy *gb){
    return bus_read(gb, gb->cpu.PC++);
}

/* Replacing the potentially 1k+ line switch case that was 62 lines after 6
 * op-codes already */
int gameboy_step(Gameboy *gb)
{
    uint64_t cycles;
    /* Here we will have some clock/cycles logic to step through, I dont think
     * i have to worry about that before i start with the graphics */
    printf("0x%.4x - ", gb->cpu.PC);
    bool enable_ime_after = gb->cpu.ime_enable_pending;
    uint8_t opcode = fetch(gb);
    printf("0x%.2x\n", opcode);
    cycles = decoder(gb, opcode);
    if (enable_ime_after) {
        gb->cpu.IME = true;
        gb->cpu.ime_enable_pending = false;
    }
    return cycles;
}
