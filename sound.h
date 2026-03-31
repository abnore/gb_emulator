#ifndef SOUND_H
#define SOUND_H

/* The simplest audio engine thinkable using the Core Audio API, AudioToolBox:
 * <https://developer.apple.com/documentation/audiotoolbox/audio-unit-v2-c-api>
 * since AudioUnit is deprecated, apple say to use the Audio Unit types in
 * Audio Toolbox instead, specifically the v2 C api.
 *
 * This API is a nightmare to work with. The docs separate into smaller chunks,
 * making searching and reading a pain. Types arent explained at the same place
 * they are used, and it is documented in Swift.
 *
 * Reusing some old code from when i was researching miniaudio.h, raylibs
 * backend audio driver. Many fine examples, and really well documented.
 *
 * The gameboy will generate a stream of PCM samples which the playback backend
 * will retrieve and play, for now its enough to play a single stream of samples
 * */
#include <stdint.h>
#include <AudioToolbox/AudioToolbox.h>

/* Simple PCM format for the test in emulator.c this may have to change later */
#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_FREQ        330
#define AUDIO_SECONDS     2
#define AUDIO_AMPLITUDE   3000

/* CoreAudioBaseTypes.h

typedef struct {
    UInt32              mNumberChannels;
    UInt32              mDataByteSize;
    void* __nullable    mData;
} AudioBuffer;

typedef struct {
    uint32_t num_buffers;
    Audio_Buffer buffer[];
}AudioBufferList;
*/

/* An AudioComponent is a void*
 * */

/* AudioComponent.h
#pragma pack(push, 4)
typedef struct AudioComponentDescription {
    OSType              componentType;
    OSType              componentSubType;
    OSType              componentManufacturer;
    UInt32              componentFlags;
    UInt32              componentFlagsMask;
} AudioComponentDescription;
#pragma pack(pop)
*/

typedef struct {
    /* 'AudioComponentInstance  _Nullable * _Nonnull'
     * (aka 'struct ComponentInstanceRecord **'
     * An opaque pointer into apples worls, aka like my window
     * AudioUnit is a synonym of that */
    AudioUnit   unit;
    uint8_t *   buffer;
    uint32_t    buffer_size;
    uint32_t    samples_amount;
    uint32_t    step;
    int32_t     read_pos;
    float       volume;
    uint8_t     num_channels;
    int8_t      dir;
    uint8_t     playing;
} Sound;

enum {
    FORWARD = 1,
    REWIND = -1,
};

/* the prototype of the callback proc that is used to render input
 * AUComponent.h
typedef OSStatus (*AURenderCallback)(void *	inRefCon,
        AudioUnitRenderActionFlags *	ioActionFlags,
        const AudioTimeStamp *			inTimeStamp,
        UInt32							inBusNumber,
        UInt32							inNumberFrames,
        AudioBufferList * __nullable	ioData) CA_REALTIME_API;
*/

Sound *init_audio(void);
void stop_audio(Sound *sound);
void free_audio(Sound *sound);

void play_pause_sound(Sound *sound);
void stop_sound(Sound *sound);
void volume_down(Sound *sound);
void volume_up(Sound *sound);

void rewind_sound(Sound *sound);
void fforward_sound(Sound *sound);

void volume_down(Sound *sound);
void volume_up(Sound *sound);

void make_test_tone(Sound *sound);

/* Redefines to fit my style and readability better. Also, doing this forces
 * me to read the docs and actually find all the types and header files on my
 * system
 *
 * AU - Audio Unit
 *
 * The enums and defines are spread across different headers
 *
 * An AudioUnit is also just a type of AudioComponent, here:
 *      typedef AudioComponentInstance AudioUnit;
 *
 * ACD type+subtype+manufactor AUComponent.h */
#define AU_TYPE_OUTPUT              'auou'
#define AU_TYPE_GENERATOR           'augn'
#define AU_SUBTYPE_DEFAULT_OUTPUT   'def '
#define AU_SUBTYPE_SYSTEM_OUTPUT    'sys '
#define AU_MFR_APPLE                'appl'

/* Format - CoreAudioBaseTypes.h */
#define AU_FMT_LINEAR_PCM           'lpcm'
#define AU_FMT_FLOAT                (1U << 0)
#define AU_FMT_BIGENDIAN            (1U << 1)
#define AU_FMT_SIGNED_INT           (1U << 2)
#define AU_FMT_PACKED               (1U << 3)

/* Property - AudioUnitProperties.h */
#define AU_PROP_STREAM_FORMAT       8
#define AU_PROP_SET_RENDER_CALLBACK 23

/* Scope - AudioUnitProperties.h */
#define AU_SCOPE_GLOBAL             0
#define AU_SCOPE_INPUT              1
#define AU_SCOPE_OUTPUT             2

#endif // SOUND_H
