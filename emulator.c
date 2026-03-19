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
    Sound *s = get_window_user_data(w);

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
        case CANOPY_KEY_V: WARN("select - pause"); pause_audio(s); break;
        case CANOPY_KEY_B: WARN("start - play"); play_audio(s); break;
        case CANOPY_KEY_J: WARN("B - stop"); stop_audio(s); break;
        case CANOPY_KEY_K: WARN("A"); break;
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

void test_pattern(uint32_t *buf)
{
    for(int i=0; i < HEIGHT * WIDTH; ++i){
        buf[i] = dmg_palette[0];
    }
    for (int y = HEIGHT/2 - 10; y < HEIGHT/2 + 10; ++y) {
        for (int x = WIDTH/2 - 40 ; x < WIDTH/2 + 40; ++x) {
            int i = y * WIDTH + x;

            buf[i] = dmg_palette[3];
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

    if (!samples) {
        FATAL("Failed to allocate audio test buffer");
        exit(1);
    }

    sound->buffer = (uint8_t *)samples;
    sound->buffer_size = frames * sizeof(int16_t);
    sound->read_pos = 0;

    float phase = 0.0f;
    float start_freq = 220.0f;
    float end_freq   = 660.0f;
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

    rect src = { 0, 0, WIDTH, HEIGHT };
    rect dst = { 0, 0, WIDTH * SCALE, HEIGHT * SCALE };

    /* --- Windowing stuff here, timer for fps and frame etc --- */
    Window *w = create_window("Gameboy Emulator - DMG-01", WIDTH*SCALE, HEIGHT*SCALE,
                                  CANOPY_WINDOW_STYLE_TITLED    |
                                  CANOPY_WINDOW_STYLE_CLOSABLE  |
                                  CANOPY_WINDOW_STYLE_MINIATURIZABLE
                                  );
    Sound *sound = init_audio();
    init_timer();
    set_icon(ICON);

    framebuffer *fb = get_framebuffer(w); // Here we will draw!
    clear_framebuffer(fb, dmg_palette[DMG_SHADE_1]);
    set_window_user_data(w, sound);

    make_test_tone(sound);
    test_pattern(gb_fb);
    blit_scaled_u32(fb, gb_fb, WIDTH, HEIGHT, src, dst);

    /* Using my custom event system to handle key inputs */
    set_callback_key(controller);

    /* --- Here we get the rom and is ready to go --- */
    init_clock();
    Gameboy gb = gb_init();
    load_cartridge(argv[1], &gb);
    // play_audio(sound);

    /* Starting to check opcodes and investigating running the ROM */
    while( !window_should_close(w) )
    {
        dispatch_events(w);
        //gameboy_step(&gb);

        if (should_render_frame()) {
            // swap_backbuffer(w,bf);
            present_buffer(w);
        }
    }

    free_audio(sound);
    free_window(w);
    remove_cartridge(&gb);
    shutdown_log();
    return 0;
}
