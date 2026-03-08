#ifndef GB_CPU_H
#define GB_CPU_H

#include <stdint.h>
#include <stdbool.h>

/* The cpu does the fetch - decode - execute cycle */

#define MCLOCK 4194304      // Hz (4.194304 MHz)
#define SCLOCK	MCLOCK>>2   // 1/4 av the master clock



/* The CPU contains registers that stores data based on op-codes */
typedef struct{

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
char vram[8*1024];
char wram[8*1024];

#endif // GB_CPU_H
