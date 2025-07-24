#include "emu.h"
#include "cpu.h"
#include <string.h>

uint8_t memory[MEMORY_SIZE];

void init_memory() {
    memset(memory, 0, MEMORY_SIZE);
}

void load_rom(const char* filename) {
    FILE* romFile = fopen(filename, "rb");
    if (!romFile) {
        printf("Error reading ROM. Please check ROM file\n");
        return;
    }
    fread(memory, 1, 0x8000, romFile); //Stores the ROM to index: 0 to 0x7FFF
    fclose(romFile);
}