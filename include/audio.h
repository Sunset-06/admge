#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <stdint.h>
#include "cpu.h"

extern SDL_AudioDeviceID audio_device;
extern bool init_audio(APU *apu);
extern void destroy_audio();

#endif
