#include "cpu.h"

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

byte read_bus(uint16_t address){
    (void)address;
    return 0;
}

/* low 4 bits on flags register, and the flag itself, are always all 0,
 * except for when we unset on the flag (inverse)
 * Therefore we check if f is 0x10,0x20,0x40 or 0x80 to make sure, and
 * also reset the last bits on the register to be sure */

void set_flag(flag f, uint8_t *reg){
    if (f & 0xf) return;
    *reg |= (uint8_t)f;
    *reg &= 0xf0;
}
void unset_flag(flag f, uint8_t *reg){
    if (f & 0xf) return;
    *reg &= ~(uint8_t)f;
    *reg &= 0xf0;
}
int test_flag(flag f, uint8_t reg){
    return reg & f;
}
