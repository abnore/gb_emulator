#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <blackbox.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "rom.h"
#include "gameboy.h"

/* xxd -i -s 0x104 -l 48 ROM/Tetris.gb produces this, althoug i changed the
 * names
 */
const unsigned char tetris_logo_rom[] = {
  0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, 0x03, 0x73, 0x00, 0x83,
  0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e,
  0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63,
  0x6e, 0x0e, 0xec, 0xcc, 0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e
};

const int tetris_logo_rom_len = 48;

/* Using mmap for simplicity, since i can create a simple uint8_t* to the data,
 * instead of using fseek etc, and also skipping a read to a large buffer.
 * When reading from the ROM it can be a lot of random jumps, which a memory
 * mapped I/O should be efficient at. It probably does not matter at this level.
 */
void load_cartridge(const char *path, void *gb)
{
    Gameboy *g = gb;
    uint8_t *r;
    struct stat st;

    int fd = open(path, O_RDONLY);
    if (fd < 0)
        goto err_out;

    fstat(fd, &st); // We know the file exists, checks in main

    r = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (r == MAP_FAILED)
        goto err_out;

    close(fd);

    g->bus.rom_size = st.st_size;

    TRACE("Loaded cartridge");

    if (r[0x0134+0xf] == 0){ // Should be less then 16 bytes, and 0 padded
        INFO("Game title: %s", (char*)&r[0x0134]);
    } else {
        ERROR("Unsafe to print title");
    }
    uint8_t checksum = 0;
    for (uint16_t address = 0x0134; address <= 0x014C; address++) {
        checksum = checksum - r[address] - 1;
    }
    TRACE("checksum is 0x%.2x, computed is: 0x%.2x", r[0x014d], checksum);
    TRACE("global checksum is 0x%.4x, this is not checked",
         *(uint16_t*)&r[0x014e]);

    int ret = memcmp(tetris_logo_rom, &r[0x104], tetris_logo_rom_len);
    TRACE("compared rom logo with , %i", ret);

    g->bus.rom = r;
    return;

err_out:
    if (fd >= 0) close(fd);
}

/* So far little cleanup needed - Perhaps more later */
void remove_cartridge(void *gb)
{
    Gameboy *g = gb;
    munmap(g->bus.rom, g->bus.rom_size);
}
