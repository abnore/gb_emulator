#ifndef GB_CPU_H
#define GB_CPU_H

#include <stdint.h>
#include <stdbool.h>

/* The cpu does the fetch - decode - execute cycle */

#define MCLOCK 4194304      // Hz (4.194304 MHz)
#define SCLOCK	MCLOCK>>2   // 1/4 av the master clock

typedef uint8_t byte;

/* The CPU contains registers that stores data based on op-codes */
typedef struct{

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
void set_flag(flag f, uint8_t *reg);
void unset_flag(flag f, uint8_t *reg);
int test_flag(flag f, uint8_t reg);

/* The memory elements are two 8KiB registers, Work and Video RAM */
byte wram[0x2000];  // $C000-$DFFF in two 4k segments
byte hram[0xf];     // $FF80-$FFFE
byte IE, IF;
/*
Memory Map
The Game Boy has a 16-bit address bus, which is used to address ROM, RAM, and I/O.

Start	End	Description	Notes
0000	3FFF	16 KiB ROM bank 00	From cartridge, usually a fixed bank
4000	7FFF	16 KiB ROM Bank 01–NN From cartridge, switchable bank via mapper (if any)
A000	BFFF	8 KiB External RAM	From cartridge, switchable bank if any
C000	CFFF	4 KiB Work RAM (WRAM)
D000	DFFF	4 KiB Work RAM (WRAM)	In CGB mode, switchable bank 1–7
FE00	FE9F	Object attribute memory (OAM)
FF00	FF7F	I/O Registers
FF80	FFFE	High RAM (HRAM)
FFFF	FFFF	Interrupt Enable register (IE)
*/

#endif // GB_CPU_H
