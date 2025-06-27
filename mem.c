#include "mem.h"

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
    fread(memory, 1, 0x8000, romFile);
    fclose(romFile);
}

uint8_t read_memory(uint16_t address) {
    if (address < 0x8000) {
        return memory[address];
    }
    else if (address >= 0x8000 && address < 0xA000) {
        return memory[address];
    }
    else if (address >= 0xC000 && address < 0xE000) {
        return memory[address];
    }
    else if (address >= 0xFF00 && address < 0xFF80) {
        return memory[address];
    }
    else if (address >= 0xFE00 && address < 0xFEA0) {
        return memory[address];
    }
    else if (address == 0xFFFF) {
        return memory[address];
    }

    return 0;
}

void write_memory(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        memory[address] = value;
    }
    else if (address >= 0x8000 && address < 0xA000) {
        memory[address] = value;
    }
    else if (address >= 0xC000 && address < 0xE000) {
        memory[address] = value;
    }
    else if (address >= 0xFF00 && address < 0xFF80) {
        memory[address] = value;
    }
    else if (address >= 0xFE00 && address < 0xFEA0) {
        memory[address] = value;
    }
    else if (address == 0xFFFF) {
        memory[address] = value;
    }
}
