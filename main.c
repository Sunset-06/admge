#include "include/emu.h"
#include "include/cpu.h"
#include "include/mem.h"

int main(int argc, char *argv[]) {
    char* inputRom = argv[1];
    load_rom(inputRom);
    /* 
    while(!quit_flag){
        fetch();
        execute();
    } */
}