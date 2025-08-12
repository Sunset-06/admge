#include "ppu.h"
#include <string.h>

void ppu_init(PPU *ppu) {
    memset(ppu, 0, sizeof(PPU));
}

void ppu_step(PPU *ppu, CPU *cpu, int cycles) {
    ppu->mode_cycles += cycles;

    // Scanlines take 456 cycles
    if (ppu->mode_cycles >= 456) {
        ppu->mode_cycles -= 456;
        ppu->ly++;

        if (ppu->ly == 144) {
            // VBlank triggered
            cpu->iflag |= 0x01; // Set VBlank interrupt
        } else if (ppu->ly > 153) {
            ppu->ly = 0;
        }
    }   
}

uint8_t ppu_read(PPU *ppu, uint16_t addr) {
    if (addr >= 0x8000 && addr <= 0x9FFF) {
        return ppu->vram[addr - 0x8000];
    } else if (addr >= 0xFE00 && addr <= 0xFE9F) {
        return ppu->oam[addr - 0xFE00];
    }

    return 0xFF;
}

void ppu_write(PPU *ppu, uint16_t addr, uint8_t value) {
    if (addr >= 0x8000 && addr <= 0x9FFF) {
        ppu->vram[addr - 0x8000] = value;
    } else if (addr >= 0xFE00 && addr <= 0xFE9F) {
        ppu->oam[addr - 0xFE00] = value;
    }

}
