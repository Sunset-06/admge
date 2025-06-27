#include"include/core.h"
#include"include/cpu.h"

int main(int argc, char *argv[]) {
    char* inputRom = argv[1];
    LoadRom(inputRom);
    
    while(!quit_flag){
        fetch();
        execute();
    }
}