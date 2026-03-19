#include "sound.h"
/* The callback takes action on the sound we define, it takes inRefCon, which
 * is the struct we pass as the Sound objects. It treats the buffer as mono
 * 16-bit samples, and for each frame requested by Core Audio (Apples backend)
 * it checks if playback reached end, if so set read_pos to 0; which goes back
 * to the start. It reads one mono sample from the buffer, scales is by volume,
 * and writes it both to the left and right output before advancing the playback
 * position.
 */
static OSStatus sound_callback(void *inRefCon,
        AudioUnitRenderActionFlags *ioActionFlags,
        const AudioTimeStamp *inTimeStamp,
        uint32_t inBusNumber,
        uint32_t inNumberFrames,
        AudioBufferList *ioData)
{
    Sound *sound = inRefCon;
    /* We asked for packed stereo PCM, so Core Audio gives us one interleaved
     * buffer here. mBuffers is an array of buffers, and mBuffers[0] is the
     * first one, and will be the output. We then set our data to this array
     * and it will be sent to the speaker, LRLR etc...
     */
    int16_t *out = (int16_t *)ioData->mBuffers[0].mData;
    int16_t *samples = (int16_t *)sound->buffer;

    (void)ioActionFlags;
    (void)inTimeStamp;
    (void)inBusNumber;

    for (uint32_t i = 0; i < inNumberFrames; i++) {
        // divive by 2 because buffer size is bytes, while samples is 16bit
        if (sound->read_pos >= sound->buffer_size / 2) {
            sound->read_pos = 0;
        }

        int16_t s = samples[sound->read_pos++];
        s = (int16_t)(s * sound->volume);
        out[i * 2] = s;     // left [even]
        out[i * 2 + 1] = s; // right [odd]
    }

    return noErr;
}

/* Basic audio init boilerplate stuff. This gets the default output, sets PCM
 * format, hooks in the callback and initializes the unit.
 */
Sound *init_audio(void)
{
    Sound *sound = calloc(1, sizeof(Sound));
    sound->volume = 1.0f;

    AudioComponentDescription acd = {
        .componentType = kAudioUnitType_Output,
        .componentSubType = kAudioUnitSubType_DefaultOutput,
        .componentManufacturer = kAudioUnitManufacturer_Apple,
        .componentFlags = 0,
        .componentFlagsMask = 0,
    };

    AudioComponent output_component = AudioComponentFindNext(NULL, &acd);
    AudioComponentInstanceNew(output_component, &sound->unit);

    AudioStreamBasicDescription audio_descriptor = {
        .mSampleRate = 48000,
        .mFormatID = kAudioFormatLinearPCM,
        .mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked,
        .mFramesPerPacket = 1,
        .mChannelsPerFrame = 2,
        .mBitsPerChannel = 16,
        .mBytesPerFrame = 4,
        .mBytesPerPacket = 4,
    };

    AudioUnitSetProperty(sound->unit,
            kAudioUnitProperty_StreamFormat,
            kAudioUnitScope_Input,
            0,
            &audio_descriptor,
            sizeof(audio_descriptor));

    AURenderCallbackStruct render_callback = {
        .inputProc = sound_callback,
        .inputProcRefCon = sound,
    };

    AudioUnitSetProperty(sound->unit,
            kAudioUnitProperty_SetRenderCallback,
            kAudioUnitScope_Global,
            0,
            &render_callback,
            sizeof(render_callback));

    AudioUnitInitialize(sound->unit);
    return sound;
}

void free_audio(Sound *sound)
{
    if (!sound)
        return;

    AudioUnitUninitialize(sound->unit);
    AudioComponentInstanceDispose(sound->unit);
    if(sound->buffer) free(sound->buffer);
    free(sound);
}

void play_audio(Sound *sound)
{
    AudioOutputUnitStart(sound->unit);
}

void stop_audio(Sound *sound)
{
    AudioOutputUnitStop(sound->unit);
}
