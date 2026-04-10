#include "platform.h"
#include "ui.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
SDL_Texture* outer_shell = NULL;

bool init_screen(int scale) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow(
        "admge",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        900, 600,
        SDL_WINDOW_SHOWN
    );
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return false;

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT
    );
    if (!texture) return false;

    printf("before img\n");

    outer_shell = IMG_LoadTexture(renderer, "assets/dmg.png");
    if (!outer_shell) {
        printf("Texture load failed: %s", IMG_GetError());
    }
    printf("All ok\n");
    ui_init(window, renderer);

    return true;
}

void present_screen(PPU *ppu) {
    ////printf("Presenting...\n");
    SDL_UpdateTexture(texture, NULL, ppu->framebuffer, SCREEN_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    // SDL_RenderCopy(renderer, texture, NU LL, NULL);
    ui_render_frame(texture, outer_shell, renderer);
    SDL_RenderPresent(renderer);
}

void destroy_screen() {
    if (texture) SDL_DestroyTexture(texture);
    if (outer_shell) SDL_DestroyTexture(outer_shell);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}
