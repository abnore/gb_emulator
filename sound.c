#include "sound.h"
/* The callback takes action on the sound we define, it takes in_sound_data,
 * which is the struct we pass as the Sound objects. It treats the buffer as
 * mono 16-bit samples, and for each frame requested by Core Audio (Apples
 * backend) it checks if playback reached end, if so set read_pos to 0; which
 * goes back to the start. It reads one mono sample from the buffer, scales is
 * by volume, and writes it both to the left and right output before advancing
 * the playback position.
 *
 * <https://developer.apple.com/documentation/audiotoolbox/aurendercallback>
 *  - The library pages are much better explained
 * <https://developer.apple.com/library/archive/documentation/MusicAudio/
 *  Conceptual/CoreAudioOverview/Introduction/Introduction.html>
 *
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
    Sound *sound = in_sound_data; // have to cast in to known structure
    /* For the test we created the audio as 16bit mono PCM, so we have to cast
     * to read it correctly. It gets duplicated for the stereo effect. How the
     * actual gameboy needs it will perhaps change this implementation later */
    int16_t *samples = (int16_t *)sound->buffer;

    AudioBuffer *audio_buf = &io_data->mBuffers[0];
    int16_t *out = audio_buf->mData; // UnsafeMutableRawPointer, same as void*


    for (uint32_t i = 0; i < in_num_frames; i++)
    {
        // Unless playing and conditions met, we output 0 to the speakers
        int16_t s = 0;

        /* If we are playing but we have reached the end, stop and reset */
        if (sound->playing && (sound->read_pos >= (int)sound->samples_amount
            || ( sound->read_pos == 0 && sound->dir == REWIND )) )
        {
            sound->playing = false;
            sound->read_pos = 0;
        } else if (sound->playing)
        {
            s = samples[sound->read_pos];
            s = (int16_t)(s * sound->volume); // mapping it to decibel first?
            sound->read_pos += sound->step * sound->dir;
        }

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
    sound->dir = FORWARD;
    sound->step = 1;
    sound->num_channels = 2;

    /* Describe the wanted components, rest of the fields is 0, mfr is only appl
     * so it can stay 0
     * Finds the next (default) component and created a new instance of it */
    AudioComponentDescription acd = {
        .componentType = AU_TYPE_OUTPUT,
        .componentSubType = AU_SUBTYPE_DEFAULT_OUTPUT
    };
    AudioComponent output_component = AudioComponentFindNext(NULL, &acd);
    AudioComponentInstanceNew(output_component, &sound->unit);

    /* Defines format through this struct */
    AudioStreamBasicDescription audio_descriptor = {
        .mSampleRate = AUDIO_SAMPLE_RATE,
        .mFormatID = AU_FMT_LINEAR_PCM,
        .mFormatFlags = AU_FMT_SIGNED_INT| AU_FMT_PACKED,
        .mFramesPerPacket = 1,
        .mChannelsPerFrame = sound->num_channels,
        .mBitsPerChannel = 16,
        .mBytesPerFrame = 4,
        .mBytesPerPacket = 4,
    };
    /* Sets the format described to the unit created, also connects the callback
     * to the unit - This is the core configuration function of the API.
     * We select the unit, and then tell it which property we are setting.
     * Then tell it scope, "which part" of the unit so to speak, then which bus,
     * 0 is the main bus/default, then a pointer to the data we are setting,
     * with the size to said data. */
    AudioUnitSetProperty(sound->unit, AU_PROP_STREAM_FORMAT, AU_SCOPE_INPUT,
            0, &audio_descriptor, sizeof(audio_descriptor));

    /* Hooks in the callback function we defined, with the data we want played
     * After that initializing the component. Ready to set the property on the
     * audio unit */
    AURenderCallbackStruct render_callback = {
        .inputProc = sound_callback,
        .inputProcRefCon = sound,
    };
    AudioUnitSetProperty(sound->unit, AU_PROP_SET_RENDER_CALLBACK, AU_SCOPE_INPUT,
                            0, &render_callback, sizeof(render_callback));

    AudioUnitInitialize(sound->unit);
    AudioOutputUnitStart(sound->unit);
    return sound;
}

void free_audio(Sound *sound)
{
    if (!sound)
        return;

    if (sound->playing) stop_sound(sound);

    AudioOutputUnitStop(sound->unit);
    AudioUnitUninitialize(sound->unit);
    AudioComponentInstanceDispose(sound->unit);
    if(sound->buffer) free(sound->buffer);
    free(sound);
}

void play_pause_sound(Sound *s)
{
    s->dir = FORWARD;
    s->step = 1;

    if( s->playing ){
        s->playing = false;
    } else {
        s->playing = true;
    }
}

void stop_sound(Sound *s)
{
    s->playing = false;
    s->read_pos = 0;
}

void rewind_sound(Sound *sound)
{
    if( sound->dir == REWIND ) sound->step++;
    sound->dir = REWIND;

    if (sound->step >= 6) sound->step = 1;
}

void fforward_sound(Sound *sound)
{
    if( sound->dir == FORWARD ) sound->step++;
    sound->dir = FORWARD;

    if (sound->step >= 6) sound->step = 1;
}

/* Linear steps for now while putting together the engine, it should be based on
 * logarithms by mapping it to a gain curve using decibels. That is the way it
 * works best for our ears, but can figure that out later, and does it belong
 * here or in the callback?
 * So far clamping to 0 and 1, stepping 0.02, which gives smooth stepping, and
 * dealing with decibels later.*/
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


/* Stolen from a audio test. Just makes a pulse wave and sweeps the pitch up.
 * Simple way of checking if the engine works. For now its good enough
 */
void make_test_tone(Sound *sound)
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

