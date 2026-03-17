#ifndef GB_H
#define GB_H
#include <stdint.h>
#include <stdbool.h>

#include "clock.h"
#include "rom.h"

typedef struct CPU
{
    bool halted;
    /* IME is a flag internal to the CPU that controls whether any interrupt
     * handlers are called, regardless of the contents of IE. IME cannot be
     * read in any way, and is modified only by ei, di and reti */
    bool IME;
    /* di enables right away, but ei sets it after the next operation.
     * So I need to flag it for *after* the next op! */
    bool ime_enable_pending;

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

} CPU;

typedef struct {
    uint8_t *rom;
    off_t rom_size;

    uint8_t wram[0x2000];   // C000-DFFF
    uint8_t hram[0x7f];     // FF80-FFFE
    uint8_t vram[0x2000];   //
    uint8_t i_enable;       // IE - FFFF
    uint8_t i_flag;         // IF - FF0F
} Bus;

typedef struct Gameboy{
    CPU cpu;
    Bus bus;
    uint64_t cycles;
} Gameboy;

Gameboy gb_init(void);

/* These helpers fetched the op code and reads and writes to the bus.
 * They take care of the addressing to which piece of hardware gets what.
 * The rest of the helpers are in decoder.c/h
 */
uint8_t fetch(Gameboy *gb);
uint8_t bus_read(Gameboy *gb, uint16_t addr);
void bus_write(Gameboy *gb, uint16_t addr, uint8_t value);

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

/* The CPU contains registers that stores data based on op-codes, returns
 * the amount of cycles performed*/
int gameboy_step(Gameboy *gb);

#endif // GB_H
