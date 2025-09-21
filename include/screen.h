#ifndef SCREEN_H
#define SCREEN_H

/* These are only functions and macros that interact with SDL and nothing else */
#include <stdbool.h>
#include <stdint.h>
#include "cpu.h"

// screen
extern bool init_screen(int scale);
extern void present_screen(PPU *ppu);
extern void destroy_screen(void);

// audio
extern bool init_audio(APU *apu);
extern void destroy_audio();

#endif
