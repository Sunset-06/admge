#include "include/emu.h"
#include "include/cpu.h"

/* Intializating everything */
uint8_t quit_flag = 0;
bool ime_enable = 0;
CPU cpu;
PPU ppu;

int main(int argc, char *argv[]) {
    char* inputRom = argv[1];
    load_rom(cpu, inputRom);

    while(!quit_flag){

    }
    /* 
    while(!quit_flag){
        fetch();
        run_pref_inst();
        // After instruction executes
        if (cpu->ime_enable) {
            cpu->ime = true;
            cpu->ime_enable = false;
        }
    } */
}