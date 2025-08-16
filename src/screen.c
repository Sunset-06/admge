#include "screen.h"
#include <SDL2/SDL.h>
#include "cpu.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

bool init_screen(int scale) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow(
        "AdmgE",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * scale,
        SCREEN_HEIGHT * scale,
        SDL_WINDOW_SHOWN
    );
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) return false;

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT
    );
    if (!texture) return false;

    return true;
}

void sdl_draw_scanline(PPU *ppu, int line) {
    printf("\n\n\n Attempting to draw scanline\n\n\n");
    
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        uint32_t pixel = ppu->framebuffer[line * SCREEN_WIDTH + x];
        printf("%08X ", pixel);  
    }
    printf("\n\n\n\n");

    SDL_Rect rect = { 0, line, SCREEN_WIDTH, 1 };
    SDL_UpdateTexture(texture, &rect,
                      &ppu->framebuffer[line * SCREEN_WIDTH],
                      SCREEN_WIDTH * sizeof(uint32_t));
}

void sdl_present() {
    printf("Presenting...\n");
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void sdl_destroy() {
    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}
