#include "bus.h"

/* Memory Map
   The Game Boy has a 16-bit address bus, which is used to address ROM, RAM,
   and I/O.

   Start End Description
   0000 3FFF 16 KiB ROM bank 00	From cartridge, usually a fixed bank
   4000 7FFF 16 KiB ROM Bank 01–NN From cartridge, switchable bank via mapper
   A000 BFFF 8 KiB External RAM	From cartridge, switchable bank if any
   C000 CFFF 4 KiB Work RAM (WRAM)
   D000 DFFF 4 KiB Work RAM (WRAM)	In CGB mode, switchable bank 1–7
   FE00 FE9F Object attribute memory (OAM)
   FF00 FF7F I/O Registers
   FF80 FFFE High RAM (HRAM)
   FFFF FFFF Interrupt Enable register (IE)
*/

/* Abstracting away the bus so that the cpu do not have to thing about
 * ROM, RAM or I/0. The CPU will just write or read from an address, and
 * the this way it is memory mapped.
 *
 * This will work for now, only touching what i have implemented so far
 */
byte bus_read(Bus *bus, uint16_t addr)
{
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

    return 0xFF;
}

void bus_write(Bus *bus, uint16_t addr, byte value)
{
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
