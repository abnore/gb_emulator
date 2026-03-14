#ifndef GB_CPU_H
#define GB_CPU_H

#include <stdint.h>
#include <stdbool.h>

#include "clock.h"
#include "rom.h"
#include "bus.h"

/* The cpu does the fetch - decode - execute cycle
 * We need registers, and op-code decoding and a step function
 */

/* The flags register, F, contains 4 flags in bit 7-4
 * https://gbdev.io/pandocs/CPU_Registers_and_Flags.html
 */
typedef enum{
    Z_F = 1<<7, // Zero flag
    N_F = 1<<6, // Subtraction flag (BCD)
    H_F = 1<<5, // Half Carry flag (BCD)
    C_F = 1<<4, // Carry flag
}flag;

/* After boot up, this is where the cartridge stores the header and first
 * instruction. We can go straight there, even though the *real* entry is,
 * in fact, 0x0
 */
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
        struct { uint8_t F; uint8_t A; }; // Low byte first, then high byte
        uint16_t AF; // Accumulator & Flags
    };
    union{
        struct { uint8_t C; uint8_t B; };
        uint16_t BC;
    };
    union{
        struct { uint8_t E; uint8_t D; };
        uint16_t DE;
    };
    union{
        struct { uint8_t L; uint8_t H; };
        uint16_t HL;
    };

    /* These have no low or hi separation */
    uint16_t SP; // Stack Pointer
    uint16_t PC; // Program Counter/Pointer

}CPU;

void cpu_step(CPU *cpu, Bus *bus);

#endif // GB_CPU_H
