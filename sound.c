#include "sound.h"
/* The callback takes action on the sound we define, it takes in_sound_data,
 * which is the struct we pass as the Sound objects. It treats the buffer as
 * mono 16-bit samples, and for each frame requested by Core Audio (Apples
 * backend) it checks if playback reached end, if so set read_pos to 0; which
 * goes back to the start. It reads one mono sample from the buffer, scales is
 * by volume, and writes it both to the left and right output before advancing
 * the playback position.
 * <https://developer.apple.com/documentation/audiotoolbox/aurendercallback>
 */
static OSStatus sound_callback(void *in_sound_data,
        AudioUnitRenderActionFlags *io_action_flags,
        const AudioTimeStamp *in_timestamp,
        uint32_t in_bus_num,
        uint32_t in_num_frames,
        AudioBufferList *io_data)
{
    /* Unused warning, keeping it simple */
    (void)io_action_flags;
    (void)in_timestamp;
    (void)in_bus_num;

    /* We asked for packed stereo PCM, so Core Audio gives us one interleaved
     * buffer here. mBuffers is an array of AudioBuffer, and mBuffers[0] is the
     * first one, and will be the output. We then write our data to this array
     * and it will be sent to the speakers, LRLR etc according to out format
     */
    Sound *sound = in_sound_data;
    AudioBuffer *a_buf = &io_data->mBuffers[0];
    int16_t *out = a_buf->mData; // UnsafeMutableRawPointer, same as void*
    int16_t *samples = (int16_t *)sound->buffer;


    for (uint32_t i = 0; i < in_num_frames; i++) {
        // divive by 2 because buffer size is bytes, while samples is 16bit
        if (sound->read_pos >= sound->buffer_size / 2) {
            sound->read_pos = 0;
        }

        int16_t s = samples[sound->read_pos++];
        s = (int16_t)(s * sound->volume); // mapping it to decibel first?
        out[i * 2]      = s;     // left [even]
        out[i * 2 + 1]  = s;     // right [odd]
    }

    return noErr;
}

/* Basic audio init boilerplate stuff. This gets the default output, sets PCM
 * format, hooks in the callback and initializes the unit.
 */
Sound *init_audio(void)
{
    Sound *sound = calloc(1, sizeof(Sound));
    sound->volume = 0.5f;

    /* Describe the wanted component, rest of the fields is 0
     * Finds the next (default) component and created a new instance of it */
    AudioComponentDescription acd = { .componentType = AU_TYPE_OUTPUT };
    AudioComponent output_component = AudioComponentFindNext(NULL, &acd);
    AudioComponentInstanceNew(output_component, &sound->unit);

    /* Defines format through this struct */
    AudioStreamBasicDescription audio_descriptor = {
        .mSampleRate = 48000,
        .mFormatID = AU_FMT_LINEAR_PCM,
        .mFormatFlags = AU_FMT_SIGNED_INT| AU_FMT_PACKED,
        .mFramesPerPacket = 1,
        .mChannelsPerFrame = 2,
        .mBitsPerChannel = 16,
        .mBytesPerFrame = 4,
        .mBytesPerPacket = 4,
    };
    /* Hooks in the callback function defined, with the data we want played and
     * sets the property. After that initializing the component. Ready to use */
    AURenderCallbackStruct render_callback = {
        .inputProc = sound_callback,
        .inputProcRefCon = sound,
    };

    /* Sets the format described to the unit created, also connects the callback
     * to the unit */
    AudioUnitSetProperty(sound->unit, AU_PROP_STREAM_FORMAT, AU_SCOPE_INPUT,
                            0, &audio_descriptor, sizeof(audio_descriptor));

    AudioUnitSetProperty(sound->unit, AU_PROP_RENDER_CALLBACK, AU_SCOPE_INPUT,
                            0, &render_callback, sizeof(render_callback));

    AudioUnitInitialize(sound->unit);
    return sound;
}

void free_audio(Sound *sound)
{
    if (!sound)
        return;

    stop_audio(sound);
    AudioUnitUninitialize(sound->unit);
    AudioComponentInstanceDispose(sound->unit);
    if(sound->buffer) free(sound->buffer);
    free(sound);
}

/* Wrappers around AudioToolBox functions. Simple at the moment. */
void play_audio(Sound *sound)
{
    AudioOutputUnitStart(sound->unit);
}
void pause_audio(Sound *sound)
{
    AudioOutputUnitStop(sound->unit);
}
void stop_audio(Sound *sound)
{
    AudioOutputUnitStop(sound->unit);
    sound->read_pos = 0;
}

/* Linear steps for now while putting together the engine, it should be based on
 * logarithms by mapping it to a gain curve using decibels. That is the way it
 * works best for our ears, but can figure that out later, and does it belong
 * here or in the callback? */
void volume_down(Sound *sound)
{
    sound->volume -= 0.02f;
    if (sound->volume < 0.0f)
        sound->volume = 0.0f;
}

void volume_up(Sound *sound)
{
    sound->volume += 0.02f;
    if (sound->volume > 1.0f)
        sound->volume = 1.0f;
}
