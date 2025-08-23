#include "emu.h"
#include "cpu.h"
#include "screen.h"

bool ime_enable = false;
bool quit_flag = false;
bool bootrom_flag = true;


char serial_log[65536];  
size_t serial_len = 0;

void dump_serial_log(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return;

    fwrite(serial_log, 1, serial_len, f);
    fclose(f);
}


void dump_vram(CPU *cpu, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        //printf("Failed to open %s for writing\n", filename);
        return;
    }

    // Dump entire VRAM (0x8000–0x9FFF)
    fwrite(&cpu->memory[0x8000], 1, 0x2000, file);
    fclose(file);
    //printf("VRAM dumped to %s\n", filename);
}

void dump_header(CPU *cpu, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        //printf("Failed to open %s for writing\n", filename);
        return;
    }

    fwrite(&cpu->memory[0x0100], 1, 0x50, file);
    fclose(file);
    //printf("Header dumped to %s\n", filename);
}

int main(int argc, char *argv[]) {
    /* Intializating everything */
    if(argc < 2){
        //printf("Wrong start, use it like this:\n admge path/to/your/rom -noboot(optional)\n Aborting...");
        return 1;
    }

    if(argc >= 3 && strcmp(argv[2], "-noboot") == 0)
        bootrom_flag = false;

    CPU cpu;
    char* inputRom = argv[1];
    start_cpu(&cpu); // This initializes Registers, CPU and PPU.
    if(!load_rom(&cpu, inputRom)){
        //printf("Aborting...");
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
                if (event.key.keysym.sym == SDLK_SPACE) {
                    dump_vram(&cpu, "dump.bin");
                    dump_header(&cpu, "header.bin");
                    dump_serial_log("serial.txt");
                }
            }
        }

        cpu_step(&cpu);
    }
    sdl_destroy();
}