#ifndef GB_CPU_H
#define GB_CPU_H

#include <stdint.h>
#include <stdbool.h>

#include "clock.h"
#include "rom.h"
#include "bus.h"

/* The cpu does the fetch - decode - execute cycle
 * We need registers, and op-code decoding and a step function */

typedef uint8_t byte;

/* The flags register, F, contains 4 flags in bit 7-4
 * https://gbdev.io/pandocs/CPU_Registers_and_Flags.html*/
typedef enum{
    Z_F = 1<<7, // Zero flag
    N_F = 1<<6, // Subtraction flag (BCD)
    H_F = 1<<5, // Half Carry flag (BCD)
    C_F = 1<<4, // Carry flag
}flag;

#define ENTRY_POINT 0x100

/* The CPU contains registers that stores data based on op-codes */
typedef struct{
    bool halted;
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

/* Maybe segregate this, but getting it up and running first, and refactoring
 * later. Once i decide on a structure, i will factor out each part
 */
typedef struct {
    const char *name;
    uint8_t cycles;
    void (*fn)(CPU *cpu, Bus *bus);
} Op;

// extern Op optable[256]; /* This might need to be exposed here */

void cpu_step(CPU *cpu, Bus *bus);

/* TODO: Maybe a list of functions isnt the best way of implementation operations.
 * Starting this way, optimizing later */

#endif // GB_CPU_H
