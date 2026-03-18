#include "graphics.h"

/* Rewritten from Picasso, to be specialized to a simple scaled buffer
 * This will act as the internal layer to supply the emulation with a real
 * way to draw, while thinking it is interacting with hardware */

static inline uint32_t *get_pixel_u32(framebuffer *fb, int x, int y) {
    return &fb->pixels[y * fb->width + x];
}

static inline void _normalize_rect(rect *r) {
    if (r->width < 0) {
        r->x += r->width;
        r->width = -r->width;
    }
    if (r->height < 0) {
        r->y += r->height;
        r->height = -r->height;
    }
}

static bool _clip_rect_to_bounds(framebuffer *fb, const rect *r, draw_bounds *db)
{
    if (!r || r->width <= 0 || r->height <= 0)
        return false;

    if (r->x >= fb->width || r->y >= fb->height ||
        r->x + r->width <= 0 || r->y + r->height <= 0)
        return false;

    db->x0 = (r->x > 0) ? r->x : 0;
    db->y0 = (r->y > 0) ? r->y : 0;
    db->x1 = (r->x + r->width  < (int)fb->width)  ? r->x + r->width  : (int)fb->width;
    db->y1 = (r->y + r->height < (int)fb->height) ? r->y + r->height : (int)fb->height;

    return true;
}

void blit_scaled_u32(framebuffer *dst, const uint32_t *src,
                     int src_width, int src_height,
                     rect src_r, rect dst_r)
{
    if (!dst || !dst->pixels || !src)
        return;
    if (src_r.width <= 0 || src_r.height <= 0)
        return;

    _normalize_rect(&src_r);
    _normalize_rect(&dst_r);

    if (src_r.x < 0) src_r.x = 0;
    if (src_r.y < 0) src_r.y = 0;
    if (src_r.x + src_r.width > src_width)
        src_r.width = src_width - src_r.x;
    if (src_r.y + src_r.height > src_height)
        src_r.height = src_height - src_r.y;

    draw_bounds bounds;
    if (!_clip_rect_to_bounds(dst, &dst_r, &bounds))
        return;

    float scale_x = (float)src_r.width / dst_r.width;
    float scale_y = (float)src_r.height / dst_r.height;

    for (int dst_y = bounds.y0; dst_y < bounds.y1; ++dst_y) {
        int rel_y = dst_y - dst_r.y;
        int src_y = src_r.y + (int)(rel_y * scale_y);

        for (int dst_x = bounds.x0; dst_x < bounds.x1; ++dst_x) {
            int rel_x = dst_x - dst_r.x;
            int src_x = src_r.x + (int)(rel_x * scale_x);

            *get_pixel_u32(dst, dst_x, dst_y) = src[src_y * src_width + src_x];
        }
    }
}
