#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL_image.h>
#include "cpu.h"
#include "ui.h"



extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *texture;
extern SDL_Texture* shell_texture;

extern bool init_screen();
extern void present_screen(PPU *ppu, CPU *cpu);
extern void destroy_screen(void);
extern void init_audio(CPU *cpu);
extern void destroy_audio();

#endif
