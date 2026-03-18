/*
 * Gameboy emulator
 *
 * Author: Andreas Nore <github.com/abnore>
 * Date: 2026
 * Licence: MIT
 *
 * Icon: https://www.shareicon.net/gameboy-190630
 * Palette: https://lospec.com/palette-list/dmg-01-accurate
 */
#include <canopy.h>
#include <blackbox.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

#include "rom.h"
#include "clock.h"
#include "gameboy.h"
#include "graphics.h"

#define SCALE 2
#define WIDTH 160
#define HEIGHT 144
#define PIXELS WIDTH*HEIGHT

static uint32_t gb_fb[PIXELS];

/* Callback function for dispatch_events(), all key input will be handle here
 * wasd is the D-pad, v and b is select and start, respecfully. they are in the
 * middle and, like the gameboy, isnt used in gameplay. J is B and K is A, they
 * are next to eachother, homerow and easy to be on. Vim motions as well */
void controller(Window *w, canopy_event_key* e)
{
    if (e->action == CANOPY_KEY_PRESS) {
        switch (e->keycode) {
        case CANOPY_KEY_ESCAPE:
            INFO("Closing the window, because you pressed esc");
            set_window_should_close(w);
            break;
        case CANOPY_KEY_W: WARN("^");break;
        case CANOPY_KEY_A: WARN("<");break;
        case CANOPY_KEY_S: WARN("v");break;
        case CANOPY_KEY_D: WARN(">");break;
        case CANOPY_KEY_V: WARN("select");break;
        case CANOPY_KEY_B: WARN("start");break;
        case CANOPY_KEY_J: WARN("B");break;
        case CANOPY_KEY_K: WARN("A");break;
        default: break;
        }
    }
}

void clear_framebuffer(framebuffer *fb, uint32_t col){
    for (int i = 0; i <  fb->width * fb->height; i++){
        fb->pixels[i] = col;
    }
}

void test_pattern(uint32_t *buf)
{
    for(int i=0; i < HEIGHT * WIDTH; ++i){
        buf[i] = dmg_palette[0];
    }
    for (int y = HEIGHT/2 - 10; y < HEIGHT/2 + 10; ++y) {
        for (int x = WIDTH/2 - 40 ; x < WIDTH/2 + 40; ++x) {
            int i = y * WIDTH + x;

            buf[i] = dmg_palette[3];
            //checkerboard with only two - & 3 for 4 colors.
            //switch ((x + y) & 1) {
            //case 0: buf[i] = WHITE;      break;
            //case 1: buf[i] = BLACK;      break;
            //case 1: buf[i] = LIGHT_GRAY; break;
            //case 2: buf[i] = DARK_GRAY;  break;
            //case 3: buf[i] = BLACK;      break;
            //}
        }
    }
}
int main(int argc, char **argv)
{
    init_log(LOG_DEFAULT);

    if (argc < 2){
        FATAL("Needs a rom file to run\nUsage: ... TODO");
        return 1;
    } else if (argc > 2){
        FATAL("Only one (1) file accepted, no flags");
        return 3;
    }
    if (access(argv[1], F_OK)){
        FATAL("%s doesn't exist",basename(argv[1]));
        return 2;
    }

    /* --- Windowing stuff here, timer for fps and frame etc --- */
    Window *w = create_window("Gameboy Emulator - DMG-01", WIDTH*SCALE, HEIGHT*SCALE,
                                  CANOPY_WINDOW_STYLE_TITLED    |
                                  CANOPY_WINDOW_STYLE_CLOSABLE  |
                                  CANOPY_WINDOW_STYLE_MINIATURIZABLE
                                  );
    init_timer();
    set_icon(ICON);
    framebuffer *fb = get_framebuffer(w); // Here we will draw!
    clear_framebuffer(fb, dmg_palette[DMG_SHADE_1]);

    test_pattern(gb_fb);
    rect src = { 0, 0, WIDTH, HEIGHT };
    rect dst = { 0, 0, WIDTH * SCALE, HEIGHT * SCALE };
    blit_scaled_u32(fb, gb_fb, WIDTH, HEIGHT, src, dst);

    /* Using my custom event system to handle key inputs */
    set_callback_key(controller);

    /* --- Here we get the rom and is ready to go --- */
    init_clock();
    Gameboy gb = gb_init();
    load_cartridge(argv[1], &gb);

    /* Starting to check opcodes and investigating running the ROM */
    while( !window_should_close(w) )
    {
        dispatch_events(w);

        //gameboy_step(&gb);
        //next_cycle();

        if (should_render_frame())
        {
            // swap_backbuffer(w,bf);
            present_buffer(w);
        }
    }

    free_window(w);
    remove_cartridge(&gb);
    shutdown_log();
    return 0;
}
