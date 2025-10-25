#ifndef SCREEN_H
#define SCREEN_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "cpu.h"

extern bool init_screen(int scale);
extern void present_screen(PPU *ppu);
extern void destroy_screen(void);
extern void init_audio(CPU *cpu);
extern void destroy_audio();

#endif
