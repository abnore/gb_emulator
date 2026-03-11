#ifndef BUS_H
#define BUS_H

#include <stdint.h>
#include "rom.h"

typedef uint8_t byte;

typedef struct {
    ROM rom;
    off_t rom_size;

    byte wram[0x2000];   // C000-DFFF
    byte hram[0x7f];     // FF80-FFFE
    byte i_enable;       // IE - FFFF
    byte i_flag;         // IF - FF0F
} Bus;

byte bus_read(Bus *bus, uint16_t addr);
void bus_write(Bus *bus, uint16_t addr, byte value);

#endif // BUS_H
