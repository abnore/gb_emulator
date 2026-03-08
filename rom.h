#ifndef ROM_H
#define ROM_H

#include <stdint.h>
#include <sys/types.h>

typedef struct _ROM {
    const char* path;
    const char* name;
    int fd;
    uint8_t *data;
    off_t size;
}ROM;

int load_cartridge(ROM *r, char *path);
void remove_cartridge(ROM *r);

#endif // ROM_H
