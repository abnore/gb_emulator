#ifndef ROM_H
#define ROM_H

#include <stdint.h>
#include <sys/types.h>

/* These functions act as a replacement for physically connecting a cartridge.
 * In real life what that is doing is connecting the 16 address pins to the bus
 * and also connecting the 8 data pins. Therefore when we address the bus, we
 * can get an 8-bit value back.
 */
extern const unsigned char tetris_logo_rom[];
extern const int tetris_logo_rom_len;

void load_cartridge(const char *path, void *gb);
void remove_cartridge(void *gb);

#endif // ROM_H
