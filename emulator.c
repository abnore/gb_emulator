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
#include <stdlib.h>

#include "rom.h"
#include "clock.h"
#include "gameboy.h"
#include "graphics.h"
#include "sound.h"

/* Callback function for dispatch_events(), all key input will be handle here
 * wasd is the D-pad, v and b is select and start, respecfully. they are in the
 * middle and, like the gameboy, isnt used in gameplay. J is B and K is A, they
 * are next to eachother, homerow and easy to be on. Vim motions as well.
 *
 * This will most likely be moved to controller.c/h
 * */
void controller(Window *w, canopy_event_key* e)
{
    Sound *s = get_window_user_data(w);

    if (e->action == CANOPY_KEY_PRESS)
    {
        switch (e->keycode)
        {
        case CANOPY_KEY_ESCAPE: set_window_should_close(w); break;
        case CANOPY_KEY_W: WARN("^");break;
        case CANOPY_KEY_A: WARN("<");break;
        case CANOPY_KEY_S: WARN("v");break;
        case CANOPY_KEY_D: WARN(">");break;
        case CANOPY_KEY_V: WARN("select"); break;
        case CANOPY_KEY_B: WARN("start - play"); play_pause_sound(s); break;
        case CANOPY_KEY_J: WARN("B - stop"); stop_sound(s); break;
        case CANOPY_KEY_K: WARN("A - rewind"); rewind_sound(s); break;
        case CANOPY_KEY_L: WARN("fforward"); fforward_sound(s); break;
        case CANOPY_KEY_Y: WARN("volume up"); volume_up(s); break;
        case CANOPY_KEY_T: WARN("volume down"); volume_down(s); break;
        default: break;
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

    /* Create the window, set the icon and initialize sound */
    block_events();
    Window *w = create_window("Gameboy Emulator - DMG-01",
                               BG_W*SCALE,
                               BG_H*SCALE,
                               CANOPY_WINDOW_STYLE_TITLED        |
                               CANOPY_WINDOW_STYLE_CLOSABLE      |
                               CANOPY_WINDOW_STYLE_MINIATURIZABLE);
    set_icon(ICON);
    Sound *sound = init_audio();

    set_window_user_data(w, sound);
    make_test_tone(sound);

    framebuffer *fb = get_framebuffer(w); // Here we will draw!
    clear_framebuffer(fb, dmg_palette[DMG_BG_GRAY]);

    rect src = { 0, 0, LCD_W, LCD_H };

    int view_x = (BG_W - LCD_W) / 2;
    int view_y = (BG_H - LCD_H) / 2;

    rect dst = {
        view_x * fb->width  / BG_W,
        view_y * fb->height / BG_H,
        LCD_W  * fb->width  / BG_W,
        LCD_H  * fb->height / BG_H
    };

    /* Using my custom event system to handle key inputs */
    set_callback_key(w, controller);

    /* Load the rom and animate logo */
    Gameboy gb = gb_init();
    load_cartridge(argv[1], &gb);

    animate_logo(w, fb, src, dst);

    /* -- Main loop -- */
    unblock_events();
    while (!window_should_close(w))
    {
        pump_messages();
        dispatch_events(w);

        if (should_render_frame()) {
            blit_scaled(fb, lcd_fb, LCD_W, LCD_H, src, dst);
            present_buffer(w);
        }
    }

    free_audio(sound);
    free_window(w);
    remove_cartridge(&gb);
    shutdown_log();
    return 0;
}
