#include "cpu.h"
#include "screen.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
static const uint32_t GAMEBOY_COLORS[4] = {
    0xFFFFFFFF, // White
    0xFFAAAAAA, // Light Gray
    0xFF555555, // Dark Gray
    0xFF000000  // Black
};

void ppu_init(PPU *ppu) {
    // Default values for LCD registers 
    ppu->lcdc = 0x91; 
    ppu->stat = 0x85; 
    ppu->scy  = 0x00; 
    ppu->scx  = 0x00; 
    ppu->ly   = 0x00; 
    ppu->lyc  = 0x00; 
    ppu->wy   = 0x00; 
    ppu->wx   = 0x00; 
    ppu->bgp  = 0xFC;
    ppu->obp0 = 0xFF; 
    ppu->obp1 = 0xFF; 

    ppu->mode_cycles = 0;
    ppu->scanline    = 0;

    // Clear framebuffer to white
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        ppu->framebuffer[i] = GAMEBOY_COLORS[0];
    }
}

void ppu_step(PPU *ppu, CPU *cpu) {
    printf("CPU cycles: %ld\n", cpu->cycles);
    ppu->mode_cycles += cpu->cycles*4;
    printf("tcycles: %d\n\n", ppu->mode_cycles);
    // Each scanline takes 456 cycles
    if(ppu->mode_cycles >= 456) {
        ppu->mode_cycles -= 456;
        printf("Decremented 456, new cycles: %d\n", ppu->mode_cycles);  
        ppu->ly++;

        printf("ly: %d\n", ppu->ly);
        if (ppu->ly < 144) {
            render_scanline(ppu, cpu);
            sdl_draw_scanline(ppu, ppu->ly);
        }

        if (ppu->ly == 144) {
            // VBlank interrupt
            printf("\n\n --------------------- Hit a VBlank. Rendering next frame.\n"); 
            cpu->iflag |= 0x01; // Set VBlank interrupt
            sdl_present();
        } 
        else if (ppu->ly > 153) {
            ppu->ly = 0;
        }
    }
}

uint8_t ppu_read(CPU *cpu, uint8_t addr) {
    return cpu->memory[addr];
}

void ppu_write(CPU *cpu, uint16_t addr, uint16_t value) {
    cpu->memory[addr] = value;

    if (addr == 0xFF40) {
        // LCDC changed, handle LCD on/off state if needed
    }
    else if (addr == 0xFF46) {
        // DMA transfer
        uint16_t src = value << 8;
        for (int i = 0; i < 0xA0; i++) {
            cpu->memory[0xFE00 + i] = cpu->memory[src + i];
        }
    }
}

void render_scanline(PPU *ppu, CPU *cpu) {
    int row_start = ppu->ly * SCREEN_WIDTH;

    printf("rendering scanline : %d\n\n", row_start);
    
    uint16_t tile_map_base = (ppu->lcdc & 0x08) ? 0x9C00 : 0x9800;
    bool signed_indexing = !(ppu->lcdc & 0x10);
    uint16_t tile_data_base = signed_indexing ? 0x8800 : 0x8000;

    uint8_t scrolled_y = (ppu->scy + ppu->ly) & 0xFF;
    uint8_t tile_row = scrolled_y / 8;

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        uint8_t scrolled_x = (ppu->scx + x) & 0xFF;
        uint8_t tile_col = scrolled_x / 8;

        uint16_t tile_index_addr = tile_map_base + (tile_row * 32) + tile_col;
        uint8_t tile_number = cpu->memory[tile_index_addr];

        uint16_t tile_addr;
        if (signed_indexing) {
            tile_addr = tile_data_base + (((int8_t)tile_number + 128) * 16);
        } else {
            tile_addr = tile_data_base + (tile_number * 16);
        }

        uint8_t tile_line = (scrolled_y % 8) * 2;
        uint8_t byte1 = cpu->memory[tile_addr + tile_line];
        uint8_t byte2 = cpu->memory[tile_addr + tile_line + 1];

        uint8_t bit = 7 - (scrolled_x % 8);
        uint8_t color_id = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);
        uint8_t palette_shade = (ppu->bgp >> (color_id * 2)) & 0x03;

        ppu->framebuffer[row_start + x] = GAMEBOY_COLORS[palette_shade];
    }
}
