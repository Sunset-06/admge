#ifndef UI_H
#define UI_H

#include <SDL.h>
#include <stdbool.h>
#include "tinyfiledialogs.h"

#ifdef __cplusplus
extern "C" {
#endif

struct CPU;
typedef struct CPU CPU;

#define DMG_WIDTH 1236
#define DMG_HEIGHT 1072
#define MGB_WIDTH 1236
#define MGB_HEIGHT 1072

extern float win_scale;

extern void ui_init(SDL_Window* window, SDL_Renderer* renderer);
extern void ui_render(SDL_Texture* emu_texture, SDL_Texture* shell_texture, SDL_Renderer* renderer, CPU *cpu);
extern void ui_handle_event(SDL_Event* event);
extern bool ui_want_capture();
extern void ui_cleanup();

#ifdef __cplusplus
}
#endif

#endif
