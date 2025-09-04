#include <SDL2/SDL.h>
#include "cpu.h" 

static SDL_AudioDeviceID audio_device;

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    APU *apu = (APU*)userdata;
    int16_t *buffer = (int16_t*)stream;
    int num_samples_to_generate = len / (sizeof(int16_t) * 2); // 2 for stereo

    // This is where you generate audio samples and fill the buffer.
    // In a real emulator, you'd have a buffer that the APU fills over time,
    // and this callback would just copy data from it.
    for (int i = 0; i < num_samples_to_generate; ++i) {
        // TODO: In your main loop, you'll call an apu_step() function that
        // generates samples and puts them in a buffer.
        // For now, let's just imagine we're getting them.
        
        int16_t left_sample = 0;  // Get a generated left sample from APU
        int16_t right_sample = 0; // Get a generated right sample from APU

        *buffer++ = left_sample;  // Left channel
        *buffer++ = right_sample; // Right channel
    }
}

/* Initializes the SDL Audio subsystem and opens an audio device. */
bool init_audio(APU *apu) {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        SDL_Log("SDL could not initialize audio! SDL_Error: %s", SDL_GetError());
        return false;
    }

    SDL_AudioSpec want, have;
    SDL_zero(want); // Fills the struct with zeros

    want.freq = 44100;                // Samples per second
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

    // Start audio playback. The callback will now be called periodically.
    SDL_PauseAudioDevice(audio_device, 0);
    return true;
}

void destroy_audio() {
    if (audio_device > 0) {
        SDL_CloseAudioDevice(audio_device);
        audio_device = 0;
    }
}