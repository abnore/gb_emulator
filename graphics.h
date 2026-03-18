#ifndef GRAPHICS_H
#define GRAPHICS_H
/* Stealing what i need, and rewriting it slightly, from Picasso, my own
 * graphical library */

#include <stdint.h>
#include <stdbool.h>
#include <canopy.h>

enum {
    DMG_SHADE_1,
    DMG_SHADE_2,
    DMG_SHADE_3,
    DMG_SHADE_4
};

static const uint32_t dmg_palette_alt[4] = {
    0xFF8EC659,
    0xFF88BA47,
    0xFF7AAB47,
    0xFF50863F
};

static const uint32_t dmg_palette[4] = {
    0xff3f9e9a,
    0xff226b49,
    0xff0b450e,
    0xff092a1b,
};
typedef struct {
    int x, y, width, height;
} rect;

typedef struct {
    int x0, y0, x1, y1;
} draw_bounds;

void blit_scaled_u32(
        framebuffer *dst, const uint32_t *src,
        int src_width, int src_height,
        rect src_r, rect dst_r);

#endif // GRAPHICS_H
