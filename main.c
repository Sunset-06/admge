#include "include/emu.h"
#include "include/cpu.h"

int main(int argc, char *argv[]) {
    char* inputRom = argv[1];
    int cycles = 0;
    load_rom(inputRom);
    /* 
    while(!quit_flag){
        fetch();
        execute();
    } */
}