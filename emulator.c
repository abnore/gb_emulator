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

#define SCALE 2
#define LCD_W 160
#define LCD_H 144

#define BG_W 256
#define BG_H 256
#define PIXELS LCD_W*LCD_H

int nintendo_logo[][48] = {
    {1,1,0,0,0,1,1,0,1,1,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
    {1,1,1,0,0,1,1,0,1,1,0,0,0,0,0,0,
     0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
    {1,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,
     0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
    {1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,
     0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,1,
     1,0,0,0,1,1,1,1,1,0,0,1,1,1,1,0},
    {1,1,0,1,0,1,1,0,1,1,0,1,1,1,0,1,
     1,0,1,1,0,1,1,0,0,1,1,0,1,1,1,0,
     1,1,0,1,1,0,0,1,1,0,1,1,0,0,1,1},
    {1,1,0,0,1,1,1,0,1,1,0,1,1,0,0,1,
     1,0,1,1,0,1,1,1,1,1,1,0,1,1,0,0,
     1,1,0,1,1,0,0,1,1,0,1,1,0,0,1,1},
    {1,1,0,0,1,1,1,0,1,1,0,1,1,0,0,1,
     1,0,1,1,0,1,1,0,0,0,0,0,1,1,0,0,
     1,1,0,1,1,0,0,1,1,0,1,1,0,0,1,1},
    {1,1,0,0,0,1,1,0,1,1,0,1,1,0,0,1,
     1,0,1,1,0,0,1,1,1,1,1,0,1,1,0,0,
     1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,0}
};
static uint32_t lcd_fb[PIXELS];

/* Callback function for dispatch_events(), all key input will be handle here
 * wasd is the D-pad, v and b is select and start, respecfully. they are in the
 * middle and, like the gameboy, isnt used in gameplay. J is B and K is A, they
 * are next to eachother, homerow and easy to be on. Vim motions as well */
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

void clear_framebuffer(framebuffer *fb, uint32_t col){
    for (int i = 0; i <  fb->width * fb->height; i++){
        fb->pixels[i] = col;
    }
}

void test_pattern(uint32_t *buf, int logo_y)
{
    const int logo_h = 8;
    const int logo_w = 48;

    int start_x = (LCD_W - logo_w) / 2;

    for (int i = 0; i < LCD_W * LCD_H; i++) {
        buf[i] = dmg_palette[DMG_SHADE_1];
    }

    for (int y = 0; y < logo_h; y++) {
        for (int x = 0; x < logo_w; x++) {
            int px = start_x + x;
            int py = logo_y + y;

            if (px < 0 || px >= LCD_W || py < 0 || py >= LCD_H)
                continue;

            if (nintendo_logo[y][x]) {
                buf[py * LCD_W + px] = dmg_palette[DMG_SHADE_4];
            }
        }
    }
}

/* Stolen from a audio test. Just makes a pulse wave and sweeps the pitch up.
 * Simple way of checking if the engine works. For now its good enough
 */
static void make_test_tone(Sound *sound)
{
    uint32_t frames = AUDIO_SAMPLE_RATE * AUDIO_SECONDS;
    int16_t *samples = malloc(frames * sizeof(int16_t));

    sound->buffer = (uint8_t *)samples;
    sound->buffer_size = frames * sizeof(int16_t);
    sound->samples_amount = sound->buffer_size / 2;
    sound->read_pos = 0;

    float phase = 0.0f;
    float start_freq = 110.0f;
    float end_freq   = 880.0f;
    float duty = 0.25f;

    for (uint32_t i = 0; i < frames; i++) {
        float t = (float)i / (float)frames;
        float freq = start_freq + (end_freq - start_freq) * t;

        phase += freq / (float)AUDIO_SAMPLE_RATE;
        if (phase >= 1.0f)
            phase -= 1.0f;

        samples[i] = (phase < duty) ? AUDIO_AMPLITUDE : -AUDIO_AMPLITUDE;
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


    /* --- Windowing stuff here --- */
    Window *w = create_window("Gameboy Emulator - DMG-01", BG_W*SCALE, BG_H*SCALE,
                                  CANOPY_WINDOW_STYLE_TITLED    |
                                  CANOPY_WINDOW_STYLE_CLOSABLE  |
                                  CANOPY_WINDOW_STYLE_MINIATURIZABLE
                                  );
    set_icon(ICON);
    Sound *sound = init_audio();

    set_window_user_data(w, sound);
    make_test_tone(sound);
    INFO("Buffer size %i", sound->buffer_size);

    framebuffer *fb = get_framebuffer(w); // Here we will draw!
    clear_framebuffer(fb, dmg_palette[DMG_BG_GRAY]);

    int final_logo_y = (LCD_H - 8) / 2;
    int logo_y = -8;   // start above the LCD

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

    /* --- Here we get the rom and is ready to go --- */
    Gameboy gb = gb_init();
    load_cartridge(argv[1], &gb);

    int anim_counter = 0;
    while (!window_should_close(w))
    {
        pump_messages();
        dispatch_events(w);

        if (should_render_frame()) {
            anim_counter++;

            if ((anim_counter % 3) == 0 && logo_y < final_logo_y) {
                logo_y += 1;
                if (logo_y > final_logo_y)
                    logo_y = final_logo_y;
            }

            test_pattern(lcd_fb, logo_y);
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
