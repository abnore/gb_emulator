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
void pause_audio(Sound *sound);

void volume_down(Sound *sound);
void volume_up(Sound *sound);

/* Redefines to fit my style and readability better
 * AU - Audio Unit
 *
 * Type */
#define AU_TYPE_OUTPUT            kAudioUnitType_Output
#define AU_SUBTYPE_DEFAULT_OUTPUT kAudioUnitSubType_DefaultOutput
#define AU_MFR_APPLE              kAudioUnitManufacturer_Apple

/* Format */
#define AU_FMT_LINEAR_PCM         kAudioFormatLinearPCM
#define AU_FMT_SIGNED_INT         kAudioFormatFlagIsSignedInteger
#define AU_FMT_PACKED             kAudioFormatFlagIsPacked

/* Property */
#define AU_PROP_STREAM_FORMAT     kAudioUnitProperty_StreamFormat
#define AU_PROP_RENDER_CALLBACK   kAudioUnitProperty_SetRenderCallback

/* Scope */
#define AU_SCOPE_INPUT            kAudioUnitScope_Input
#define AU_SCOPE_GLOBAL           kAudioUnitScope_Global

#endif // SOUND_H
