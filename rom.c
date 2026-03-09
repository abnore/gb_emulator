#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <blackbox.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <libgen.h>

#include "rom.h"

/*  xxd -i -s 0x104 -l 48 ROM/Tetris.gb produces this */

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
 * */
int load_cartridge(ROM *rom, char *path){

    rom->path = path;
    rom->name = basename(path);

    if (access(rom->path, F_OK)){
        FATAL("%s doesn't exist", rom->name);
        return(1);
    }

    INFO("Opening: %s", rom->name);
    rom->fd = open(rom->path, O_RDONLY);

    struct stat st;
    stat(rom->path, &st);
    rom->size = st.st_size;
    rom->data = mmap(NULL, rom->size, PROT_READ, MAP_PRIVATE, rom->fd, 0);

    return 0;
}
void remove_cartridge(ROM *r){
    munmap(r->data, r->size);
    close(r->fd);
}
