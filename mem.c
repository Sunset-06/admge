#include "emu.h"
#include "cpu.h"
#include <string.h>

uint8_t memory[MEMORY_SIZE];

void init_memory(CPU *cpu) {
    memset(cpu->memory, 0, MEMORY_SIZE);
}

void load_rom(CPU *cpu, const char* filename) {
    FILE* romFile = fopen(filename, "rb");
    if (!romFile) {
        printf("Error reading ROM. Please check ROM file\n");
        return;
    }
    fread(cpu->memory, 1, 0x8000, romFile); //Stores the ROM to index: 0 to 0x7FFF
    fclose(romFile);
}

int8_t read8(CPU *cpu, uint16_t addr) {
    if (addr >= 0xE000 && addr <= 0xFDFF) {
        // Echo RAM maps to WRAM (0xC000 - 0xDDFF)
        return cpu->memory[addr - 0x2000];
    }
    return cpu->memory[addr];
}

uint16_t read16(struct CPU* cpu, uint16_t addr) {
    // Little-endian: lower byte first
    return read8(cpu, addr) | (read8(cpu, addr + 1) << 8);
}

void write8(CPU *cpu, uint16_t addr, uint8_t value) {
    if (addr >= 0xE000 && addr <= 0xFDFF) {
        // Echo RAM
        cpu->memory[addr - 0x2000] = value;
        return;
    }

    cpu->memory[addr] = value;
}

void write16(CPU *cpu, uint16_t addr, uint16_t value) {
    // Little-endian: lower byte first
    write8(cpu, addr, value & 0xFF);         // Lower byte
    write8(cpu, addr + 1, (value >> 8) & 0xFF); // Upper byte
}