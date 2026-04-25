#include "platform.h"
#include "ui.h"
#include "emu.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
SDL_Texture* outer_shell = NULL;

bool init_screen() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return false;
    }

    int win_w = (current_mode == MGB) ? MGB_WIDTH : DMG_WIDTH;
    int win_h = (current_mode == MGB) ? MGB_HEIGHT : DMG_HEIGHT;


    window = SDL_CreateWindow(
        "admge",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        win_w*win_scale, win_h*win_scale,
        SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS
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

    outer_shell = (current_mode == MGB)? IMG_LoadTexture(renderer, "assets/mgb.png") : IMG_LoadTexture(renderer, "assets/dmg.png");
    if (!outer_shell) {
        printf("Texture load failed: %s", IMG_GetError());
    }
    printf("All ok\n");
    ui_init(window, renderer);

    return true;
}

void present_screen(PPU *ppu, CPU *cpu) {
    if (current_mode == TEST) return;
    ////printf("Presenting...\n");
    SDL_UpdateTexture(texture, NULL, ppu->framebuffer, SCREEN_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    // SDL_RenderCopy(renderer, texture, NULL, NULL);
    ui_render(texture, outer_shell, renderer, cpu);
    SDL_RenderPresent(renderer);
}

void destroy_screen() {
    if (texture) SDL_DestroyTexture(texture);
    if (outer_shell) SDL_DestroyTexture(outer_shell);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}
