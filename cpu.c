#include "cpu.h"

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
