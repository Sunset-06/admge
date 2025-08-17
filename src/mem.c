#include "emu.h"
#include "cpu.h"
#include <stdio.h>
#include <string.h>

//#define BOOT_ROM "./bootrom/bootix_dmg.bin"
#define BOOT_ROM "./bootrom/dmg_boot.bin"

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
    if (addr >= 0xFF10 && addr <= 0xFF3F) return 0xFF; // NO Sound 
    if (addr == 0xFF00) return 0xCF; // No buttons

    if (cpu->bootrom_flag && addr < 0x0100) {
        return cpu->bootrom[addr];
    }

    // VRAM access control
    if (addr >= 0x8000 && addr <= 0x9FFF) {
        // Only block if display is enabled AND in mode 3
        if ((cpu->ppu.lcdc & 0x80) && ((cpu->ppu.stat & 0x03) == 0x03)) {
            return 0xFF;
        }
        return cpu->memory[addr];
    }


    // Echo RAM
    if (addr >= 0xE000 && addr <= 0xFDFF) {
        return cpu->memory[addr - 0x2000];
    }

    // OAM access blocked during modes 2+3
    if ((addr >= 0xFE00 && addr <= 0xFE9F) && 
        ((cpu->ppu.stat & 0x03) >= 0x02)) {
        return 0xFF;
    }

    // Unusable memory
    if (addr >= 0xFEA0 && addr <= 0xFEFF) {
        return 0xFF;
    }

    // PPU registers
    if (addr >= 0xFF40 && addr <= 0xFF4B) {
        return ppu_read(cpu, addr);
    }

    return cpu->memory[addr];
}

uint16_t read16(CPU *cpu, uint16_t addr) {
    return read8(cpu, addr) | (read8(cpu, addr + 1) << 8);
}

void write8(CPU *cpu, uint16_t addr, uint8_t value) {

    // Boot ROM disable
    if (addr == 0xFF50 && cpu->bootrom_flag) {
        cpu->bootrom_flag = false;
        return;
    }

    // Echo RAM
    if (addr >= 0xE000 && addr <= 0xFDFF) {
        cpu->memory[addr - 0x2000] = value;
        return;
    }

    // OAM access blocked during modes 2+3
    if ((addr >= 0xFE00 && addr <= 0xFE9F) && 
        ((cpu->ppu.stat & 0x03) >= 0x02)) {
        return;
    }

    // Unusable memory
    if (addr >= 0xFEA0 && addr <= 0xFEFF) {
        return;
    }

    // PPU registers
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
    cpu->sp -= 2;
    write16(cpu, cpu->sp, value);
}

uint16_t stack_pop(CPU *cpu) {
    uint16_t value = read16(cpu, cpu->sp);
    cpu->sp += 2;
    return value;
}