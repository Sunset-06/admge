#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <stdbool.h>
#include "cpu.h"

// Screen dimensions for the Game Boy
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

#define VRAM_SIZE 0x2000  // 8KB
#define OAM_SIZE 0xA0     // 160 bytes

typedef struct {
    uint8_t vram[VRAM_SIZE]; // Tile and BG map data
    uint8_t oam[OAM_SIZE];   // Sprite attribute table

    // LCD Registers
    uint8_t lcdc; // LCD Control
    uint8_t stat; // LCD Status
    uint8_t scy;  // Scroll Y
    uint8_t scx;  // Scroll X
    uint8_t ly;   // LCD Y-coordinate
    uint8_t lyc;  // LY Compare
    uint8_t wy;   // Window Y
    uint8_t wx;   // Window X
    uint8_t bgp;  // BG Palette
    uint8_t obp0; // Object Palette 0
    uint8_t obp1; // Object Palette 1

    // PPU timing
    int mode_cycles;
    int scanline;

    // Framebuffer (RGB or grayscale pixels)
    uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
} PPU;

// Initialize the PPU state
void ppu_init(PPU *ppu);

// Update the PPU per CPU cycle
void ppu_step(PPU *ppu, CPU *cpu, int cycles);

// Expose VRAM and OAM read/write functions
uint8_t ppu_read(PPU *ppu, uint16_t addr);
void ppu_write(PPU *ppu, uint16_t addr, uint8_t value);

// Draw the final frame to screen
void ppu_render_frame(PPU *ppu, SDL_Surface *surface);

#endif
