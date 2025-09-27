#include "cpu.h" 

SDL_AudioDeviceID audio_device;

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    printf("used callback!");
    APU *apu = (APU*)userdata;
    int16_t *op_buffer = (int16_t*)stream;
    
    int num_sample_frames = len / (sizeof(int16_t) * 2);

    SDL_LockAudioDevice(audio_device);
    
    for (int i = 0; i < num_sample_frames; ++i) {
        int16_t sample = 0; // Default to silence

        if (apu->read_pos != apu->write_pos) {
            sample = apu->internal_buffer[apu->read_pos];
            apu->read_pos = (apu->read_pos + 1) % 4096;
        }

        op_buffer[i * 2]     = sample;
        op_buffer[i * 2 + 1] = sample;
    }

    SDL_UnlockAudioDevice(audio_device);
}

bool init_audio(APU *apu) {
    printf("initing Audio thread\n");
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        SDL_Log("SDL could not initialize audio! SDL_Error: %s", SDL_GetError());
        return false;
    }

    SDL_AudioSpec want, have;
    SDL_zero(want); 

    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;       // Signed 16-bit samples, system endianness
    want.channels = 2;                // Stereo audio
    want.samples = 1024;              // Audio buffer size in samples. Lower for less latency.
    want.callback = audio_callback;
    want.userdata = apu;              // Pass our APU struct to the callback

    audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

    if (audio_device == 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
        return false;
    }

    SDL_PauseAudioDevice(audio_device, 0);
    return true;
}

void destroy_audio() {
    if (audio_device > 0) {
        SDL_CloseAudioDevice(audio_device);
        audio_device = 0;
    }
}