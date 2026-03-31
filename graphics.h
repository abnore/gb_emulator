#ifndef GRAPHICS_H
#define GRAPHICS_H
/* Stealing what i need, and rewriting it slightly, from Picasso, my own
 * graphical library */

#include <stdint.h>
#include <stdbool.h>
#include <canopy.h>

#define _ABS(a) ({                                  \
    __typeof__(a) _a = (a);                         \
    _a > 0 ? _a : -_a;                              \
})

#define _MIN(a,b) ({                                \
    __typeof__(a) _a = (a);                         \
    __typeof__(b) _b = (b);                         \
    _a < _b ? _a : _b;                              \
})

#define _MAX(a,b) ({                                \
    __typeof__(a) _a = (a);                         \
    __typeof__(b) _b = (b);                         \
    _a > _b ? _a : _b;                              \
})

#define SCALE 2
#define LCD_W 160
#define LCD_H 144
#define BG_W 256
#define BG_H 256
#define PIXELS LCD_W*LCD_H

uint32_t lcd_fb[PIXELS];

enum {
    DMG_SHADE_1,
    DMG_SHADE_2,
    DMG_SHADE_3,
    DMG_SHADE_4,
    DMG_BG_GRAY,
    DMG_SHADE_COUNT,
};

static const int nintendo_logo[][48] = {
    {1,1,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
    {1,1,1,0,0,1,1,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
    {1,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
    {1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,0,1,1,0,0,1,1,
    1,1,0,0,1,1,0,1,1,0,0,0,1,1,1,1,1,0,0,1,1,1,1,0},
    {1,1,0,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,
    0,1,1,0,1,1,1,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0,1,1},
    {1,1,0,0,1,1,1,0,1,1,0,1,1,0,0,1,1,0,1,1,0,1,1,1,
    1,1,1,0,1,1,0,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0,1,1},
    {1,1,0,0,1,1,1,0,1,1,0,1,1,0,0,1,1,0,1,1,0,1,1,0,
    0,0,0,0,1,1,0,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0,1,1},
    {1,1,0,0,0,1,1,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0,1,1,
    1,1,1,0,1,1,0,0,1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,0}
};

/* Found a slightly dimmer palette first, keeping it for testing */
static const uint32_t dmg_palette_alt[] = {
    0xFF8EC659,
    0xFF88BA47,
    0xFF7AAB47,
    0xFF50863F,
    0xff303030,
};

/* Palette from the docs, with a gray */
static const uint32_t dmg_palette[] = {
    0xff3f9e9a,
    0xff226b49,
    0xff0b450e,
    0xff092a1b,
    0xff303030,
};

typedef struct {
    int x, y, width, height;
} rect;

typedef struct {
    int x0, y0, x1, y1;
} draw_bounds;

void blit_scaled (
    framebuffer *dst,
    const uint32_t *src,
    int src_width,
    int src_height,
    rect dst_r,
    rect src_r
);

void animate_logo (
    Window *w,
    framebuffer *fb,
    rect src,
    rect dst
);

void clear_framebuffer(
    framebuffer *fb,
    uint32_t col
);

#endif // GRAPHICS_H
