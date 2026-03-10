#ifndef GB_CPU_H
#define GB_CPU_H

#include <stdint.h>
#include <stdbool.h>

#include "clock.h"

/* The cpu does the fetch - decode - execute cycle */
typedef uint8_t byte;

/* The memory elements are two 8KiB registers, Work and Video RAM */
byte wram[0x2000];  // $C000-$DFFF in two 4k segments
byte hram[0xf];     // $FF80-$FFFE
byte IE;            // $FFFF
byte IF;            // $FF0F


#define ENTRY_POINT 0x100

/* The CPU contains registers that stores data based on op-codes */
typedef struct{
    bool running;
    /* IME is a flag internal to the CPU that controls whether any interrupt
     * handlers are called, regardless of the contents of IE. IME cannot be read in
     * any way, and is modified only by ei, di and reti */
    bool IME;

    /* Registers, uses unions so I can access it as cpu.AF or cpu.A/cpu.F */
    union{
        struct { byte F; byte A; }; // Low byte first, then high byte
        uint16_t AF; // Accumulator & Flags
    };
    union{
        struct { byte C; byte B; };
        uint16_t BC;
    };
    union{
        struct { byte E; byte D; };
        uint16_t DE;
    };
    union{
        struct { byte L; byte H; };
        uint16_t HL;
    };

    /* These have no low or hi separation */
    uint16_t SP; // Stack Pointer
    uint16_t PC; // Program Counter/Pointer

}CPU;

/* The flags register, F, contains 4 flags in bit 7-4 */
typedef enum{
    Z_F = 1<<7, // Zero flag
    S_F = 1<<6, // Subtraction flag (BCD)
    H_F = 1<<5, // Half Carry flag (BCD)
    C_F = 1<<4, // Carry flag
}flag;

/* TODO: Maybe a list of functions isnt the best way of implementation operations.
 * Starting thi way, optimizing later */
void set_flag(flag f, byte *reg);
void unset_flag(flag f, byte *reg);
int test_flag(flag f, byte reg);

byte read_bus(uint16_t address);

#endif // GB_CPU_H
