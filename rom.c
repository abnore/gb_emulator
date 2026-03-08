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

/* Using mmap for simplicity, since i can create a simple char* to the data,
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

    INFO("file: %s", rom->name);
    rom->fd = open(rom->path, O_RDWR);
    struct stat st;
    stat(rom->path, &st);
    rom->size = st.st_size;
    rom->data = mmap(NULL, rom->size, PROT_READ | PROT_WRITE, MAP_SHARED, rom->fd, 0);

    return 0;
}
void remove_cartridge(ROM *r){
    munmap(r->data, r->size);
}

