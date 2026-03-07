#ifndef GB_CPU_H
#define GB_CPU_H

#include <stdint.h>

/* The cpu does the fetch - decode - execute cycle */

#define MCLOCK 4.194304 // MHz
#define SCLOCK	MCLOCK>>2 // 1/4 av the master clock

/* The CPU contains registers that stored data based on op-codes */
typedef struct{
    union{
        struct {
            uint8_t F; // low byte
            uint8_t A; // high byte
        };
        uint16_t AF; // Accumulator & Flags
    };
    union{
        struct {
            uint8_t C;
            uint8_t B;
        };
        uint16_t BC;
    };
    union{
        struct {
            uint8_t E;
            uint8_t D;
        };
        uint16_t DE;
    };
    union{
        struct {
            uint8_t L;
            uint8_t H;
        };
        uint16_t HL;
    };

    uint16_t SP; // Stack Pointer
    uint16_t PC; // Program Counter/Pointer
}CPU;

/* The flags register, F, contains 4 flags in bit 7-4 */
enum{
    Z_F = 1<<7, // Zero flag
    S_F = 1<<6, // Subtraction flag (BCD)
    H_F = 1<<5, // Half Carry flag (BCD)
    C_F = 1<<4, // Carry flag
};


/* The memory elements are tw
 * */
#endif // GB_CPU_H
