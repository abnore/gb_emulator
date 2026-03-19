#ifndef SOUND_H
#define SOUND_H

/* The simplest audio engine thinkable
 * <https://developer.apple.com/documentation/audiotoolbox/audio-unit-v2-c-api>
 *
 * This API is a nightmare to work with. Reusing some old code from when i was
 * researching miniaudio.h, raylibs backend audio driver. Many fine examples,
 * and really well documented.
 *
 * The gameboy will generate a stream of PCM samples which the playback backend
 * will retrieve and play, for now its enough to play a single stream of samples
 * */
#include <stdint.h>
#include <AudioToolbox/AudioToolbox.h>

#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_FREQ        330
#define AUDIO_SECONDS     2
#define AUDIO_AMPLITUDE   3000

typedef struct {
    AudioComponentInstance unit;
    uint8_t *buffer;
    uint32_t buffer_size;
    uint32_t read_pos;
    float volume;
} Sound;

Sound *init_audio(void);
void free_audio(Sound *sound);

void play_audio(Sound *sound);
void stop_audio(Sound *sound);

#endif // SOUND_H
