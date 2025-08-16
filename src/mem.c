#include "emu.h"
#include "cpu.h"
#include <stdio.h>
#include <string.h>

#define BOOT_ROM "./bootrom/bootix_dmg.bin"

void init_memory(CPU *cpu) {
    memset(cpu->memory, 0, MEMORY_SIZE);
}

bool load_rom(CPU *cpu, const char* filename) {
    printf("\nLoading bootrom: %s\n",BOOT_ROM);
    FILE *bootromFile = fopen(BOOT_ROM, "rb");
    if (bootromFile == NULL) {
        printf("Error: Could not load boot ROM.\n");
        return false;
    }

    FILE* romFile = fopen(filename, "rb");
    if (!romFile) {
        printf("Error reading ROM. Please check ROM file\n");
        return false;
    }

    fread(cpu->bootrom, 1, 0x0100, bootromFile);
    fread(cpu->memory, 1, 0x8000, romFile); // 0x0000 - 0x7FFF

    fclose(bootromFile);
    fclose(romFile);
    return true;
}

uint8_t read8(CPU *cpu, uint16_t addr) {
    // for reading from the bootrom
    if (addr < 0x0100 && cpu->bootrom_flag)
        return cpu->bootrom[addr];

    // Echo RAM: E000–FDFF mirrors C000–DDFF
    if (addr >= 0xE000 && addr <= 0xFDFF)
        return cpu->memory[addr - 0x2000];

    // OAM: FE00–FE9F
    if (addr >= 0xFE00 && addr <= 0xFE9F)
        return cpu->memory[addr];

    // Unusable memory: FEA0–FEFF
    if (addr >= 0xFEA0 && addr <= 0xFEFF)
        return 0xFF;

    // PPU registers: FF40–FF4B
    if (addr >= 0xFF40 && addr <= 0xFF4B)
        return ppu_read(cpu, addr);

    return cpu->memory[addr];
}

uint16_t read16(CPU *cpu, uint16_t addr) {
    return read8(cpu, addr) | (read8(cpu, addr + 1) << 8);
}

void write8(CPU *cpu, uint16_t addr, uint8_t value) {
    // switching from bootrom to memory at FF50
    if (addr == 0xFF50 && cpu->bootrom_flag) {
        cpu->bootrom_flag = false;
        return;
    }

    
    // Echo RAM
    if (addr >= 0xE000 && addr <= 0xFDFF) {
        cpu->memory[addr] = value;
        cpu->memory[addr - 0x2000] = value;
        return;
    }

    // OAM: FE00–FE9F
    if (addr >= 0xFE00 && addr <= 0xFE9F) {
        cpu->memory[addr] = value;
        return;
    }

    // Unusable memory: FEA0–FEFF
    if (addr >= 0xFEA0 && addr <= 0xFEFF) {
        return; // Ignore writes
    }

    // PPU registers: FF40–FF4B
    if (addr >= 0xFF40 && addr <= 0xFF4B) {
        ppu_write(cpu, addr, value);
        return;
    }

    cpu->memory[addr] = value;
}

void write16(CPU *cpu, uint16_t addr, uint16_t value) {
    write8(cpu, addr, value & 0xFF);
    write8(cpu, addr + 1, (value >> 8) & 0xFF);
}

// ------------------------ STACK Functions ---------------------------//

void stack_push(CPU *cpu, uint16_t value) {
    cpu->sp--;
    write8(cpu, cpu->sp, (value >> 8) & 0xFF);
    cpu->sp--;
    write8(cpu, cpu->sp, value & 0xFF);
}

uint16_t stack_pop(CPU *cpu) {
    uint8_t low = read8(cpu, cpu->sp);
    cpu->sp++;
    uint8_t high = read8(cpu, cpu->sp);
    cpu->sp++;
    return (high << 8) | low;
}
