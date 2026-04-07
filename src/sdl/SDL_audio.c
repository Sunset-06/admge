#include "cpu.h"
#include "peripherals.h"

static SDL_AudioDeviceID audio_device;

/**
 * This is the "Consumer" thread.
 * It runs completely separate from the main emulator loop.
 * It's job is to pull samples from the ring buffer and give them to SDL.
 */
static void audio_callback(void *userdata, Uint8 *stream, int len) {
    CPU *cpu = (CPU *)userdata;
    APU *apu = &cpu->apu;
    
    // Cast the stream buffer to 16-bit signed samples
    int16_t *output_buffer = (int16_t *)stream;
    
    int samples_needed = len / sizeof(int16_t);

    for (int i = 0; i < samples_needed; i++) {
        int read_pos = atomic_load(&apu->read_pos);
        int write_pos = atomic_load(&apu->write_pos);

        if (read_pos == write_pos) {
            output_buffer[i] = 0;
        } else {
            output_buffer[i] = apu->internal_buffer[read_pos];
            
            read_pos = (read_pos + 1) % AUDIO_BUFFER_SIZE;
            atomic_store(&apu->read_pos, read_pos);
        }
    }
}

void init_audio(CPU *cpu) {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        printf("Failed to initialize SDL Audio: %s\n", SDL_GetError());
        return;
    }

    SDL_AudioSpec want, have;
    SDL_zero(want);

    want.freq = SAMPLE_RATE;       // 44100 Hz
    want.format = AUDIO_S16SYS;    // 16-bit signed, native endian
    want.channels = 2;             // Stereo
    want.samples = 512;            // Buffer size for SDL, controls latency
    want.callback = audio_callback;
    want.userdata = cpu;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

    if (audio_device == 0) {
        printf("Failed to open audio device: %s\n", SDL_GetError());
        return;
    }

    SDL_PauseAudioDevice(audio_device, 0);
    printf("Audio initialized.\n");
}

void destroy_audio() {
    if (audio_device > 0) {
        SDL_CloseAudioDevice(audio_device);
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}