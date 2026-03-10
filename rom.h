#ifndef ROM_H
#define ROM_H

#include <stdint.h>
#include <sys/types.h>


extern const unsigned char tetris_logo_rom[];
extern const int tetris_logo_rom_len;

typedef uint8_t* ROM;

ROM load_cartridge(const char *path, off_t *size_out);
void remove_cartridge(ROM r, off_t size);

uint8_t read_cart(uint16_t address);

#endif // ROM_H
