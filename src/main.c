#include "emu.h"
#include "cpu.h"
#include "screen.h"


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
    
    load_rom(&cpu, inputRom);
    start_cpu(&cpu); // This initializes Registers, CPU and PPU.
    init_screen(4);
    
    while(!quit_flag){
        cpu_step(&cpu);
        if (ime_enable) {
            cpu.ime = true;
            ime_enable = false;
        }
    }
    sdl_destroy();
}