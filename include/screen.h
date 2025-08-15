#ifndef SDL_DISPLAY_H
#define SDL_DISPLAY_H

#include <stdbool.h>
#include <stdint.h>
#include "cpu.h"

extern bool init_screen(int scale);
//void sdl_draw(PPU *ppu);
extern void sdl_draw_scanline(PPU *ppu, int line);
extern void sdl_present();
extern void sdl_destroy(void);

#endif
