#ifndef SDL_DISPLAY_H
#define SDL_DISPLAY_H

#include <stdbool.h>
#include <stdint.h>
#include "ppu.h"

bool sdl_init(int scale);

void sdl_draw(PPU *ppu);

void sdl_destroy(void);

#endif
