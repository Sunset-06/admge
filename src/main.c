#include "include/emu.h"
#include "include/cpu.h"

int main(int argc, char *argv[]) {
    /* Intializating everything */
    uint8_t quit_flag = 0;
    bool ime_enable = 0;
    CPU cpu;
    PPU ppu;
    char* inputRom = argv[1];
    
    load_rom(cpu, inputRom);
    start_cpu(&cpu);
    ppu_init(&ppu);
    
    while(!quit_flag){
        
        if (ime_enable) {
            cpu.ime = true;
            ime_enable = false;
        }
    }
    /* 
    while(!quit_flag){
        fetch();
        run_pref_inst();
        // After instruction executes
    } */
}