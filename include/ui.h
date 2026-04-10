#ifndef UI_H
#define UI_H

#include <SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void ui_init(SDL_Window* window, SDL_Renderer* renderer);
extern void ui_render(SDL_Texture* emu_texture);
extern void ui_cleanup();

#ifdef __cplusplus
}
#endif

#endif