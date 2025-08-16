#include "emu.h"
#include "cpu.h"
#include "screen.h"

// epic function to make it not crash because im to lazy to poll inputs

bool ime_enable = false;
bool quit_flag = false;

int main(int argc, char *argv[]) {
    /* Intializating everything */
    if(argc != 2){
        printf("Wrong start, use it like this:\n admge path/to/your/rom\n Aborting...");
        return 1;
    }

    CPU cpu;
    char* inputRom = argv[1];
    start_cpu(&cpu); // This initializes Registers, CPU and PPU.
    if(!load_rom(&cpu, inputRom)){
        printf("Aborting...");
        return 1;
    }
    
    init_screen(4);

    while(!quit_flag){
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit_flag = true; 
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit_flag = true;  
                }
            }
        }

        cpu_step(&cpu);
        if (ime_enable) {
            cpu.ime = true;
            ime_enable = false;
        }
    }
    sdl_destroy();
}