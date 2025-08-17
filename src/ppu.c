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
    ppu->mode_cycles += cpu->cycles*4;
    // Each scanline takes 456 cycles
    if(ppu->mode_cycles >= 456) {
        ppu->mode_cycles -= 456;
        printf("Decremented 456, new cycles: %d\n", ppu->mode_cycles);  
        ppu->ly++;

        printf("ly: %d\n", ppu->ly);
        if (ppu->ly < 144) {    
        printf("ly < 144, rendering scanline now.\n\n");
            render_scanline(ppu, cpu);
        }

        if (ppu->ly == 144) {
            // VBlank interrupt
            printf("\n\n --------------------- Hit a VBlank. Rendering next frame.\n"); 
            cpu->iflag |= 0x01; // Set VBlank interrupt
            sdl_present(ppu);
        } 
        else if (ppu->ly > 153) {
            ppu->ly = 0;
        }
    }
    //printf("\nLCDC -> %d\n",ppu->lcdc);
}

uint8_t ppu_read(CPU *cpu, uint16_t addr) {
    PPU *ppu = &cpu->ppu;

    if (addr >= 0x8000 && addr <= 0x9FFF) {
        return cpu->memory[addr];
    }

    switch (addr) {
        case 0xFF40: return ppu->lcdc;
        case 0xFF41: return ppu->stat;
        case 0xFF42: return ppu->scy;
        case 0xFF43: return ppu->scx;
        case 0xFF44: return ppu->ly;   
        case 0xFF45: return ppu->lyc;
        case 0xFF47: return ppu->bgp;
        case 0xFF48: return ppu->obp0;
        case 0xFF49: return ppu->obp1;
        case 0xFF4A: return ppu->wy;
        case 0xFF4B: return ppu->wx;
    }

    return 0xFF; // unmapped
}

void ppu_write(CPU *cpu, uint16_t addr, uint8_t value) {
    PPU *ppu = &cpu->ppu;

    if (addr >= 0x8000 && addr <= 0x9FFF) {
        cpu->memory[addr] = value;
        return;
    }

    switch (addr) {
        case 0xFF40: ppu->lcdc = value; break;
        case 0xFF41: ppu->stat = value; break;
        case 0xFF42: ppu->scy  = value; break;
        case 0xFF43: ppu->scx  = value; break;
        //case 0xFF44: ppu->ly   = 0;     break; // <- This one is read only
        case 0xFF45: ppu->lyc  = value; break;
        case 0xFF47: ppu->bgp  = value; break;
        case 0xFF48: ppu->obp0 = value; break;
        case 0xFF49: ppu->obp1 = value; break;
        case 0xFF4A: ppu->wy   = value; break;
        case 0xFF4B: ppu->wx   = value; break;

        case 0xFF46: { // DMA
            uint16_t src = value << 8;
            for (int i = 0; i < 0xA0; i++) {
                cpu->memory[0xFE00 + i] = cpu->memory[src + i];
            }
            break;
        }
    }
    printf("\n\n\n VRAM:");
    for(int i=0x8000; i<0x9FFF;i++){
        printf("%02x ", cpu->memory[i]);
    }
}

void render_scanline(PPU *ppu, CPU *cpu) {
    if (!(ppu->lcdc & 0x80)) return; // Display disabled
    if (!(ppu->lcdc & 0x01)) return; // BG disabled


    uint16_t tile_map_base = 0x9800;  // Fixed for boot ROM
    uint16_t tile_data_base = 0x8000; // Unsigned indexing
    
    uint8_t scrolled_y = (ppu->scy + ppu->ly) % 256; 
    uint8_t tile_row = scrolled_y / 8;
    
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        uint8_t scrolled_x = (ppu->scx + x) % 256;
        uint8_t tile_col = scrolled_x / 8;
        
        uint16_t tile_index_addr = tile_map_base + (tile_row * 32) + tile_col;
        uint8_t tile_number = cpu->memory[tile_index_addr];
        
        uint16_t tile_addr = tile_data_base + (tile_number * 16);
        
        uint8_t tile_line = scrolled_y % 8;
        uint8_t byte1 = cpu->memory[tile_addr + tile_line*2];
        uint8_t byte2 = cpu->memory[tile_addr + tile_line*2 + 1];
        
        uint8_t bit = 7 - (scrolled_x % 8);
        uint8_t color_id = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);
        uint8_t shade = (ppu->bgp >> (color_id * 2)) & 0x03;
        
        ppu->framebuffer[ppu->ly * SCREEN_WIDTH + x] = GAMEBOY_COLORS[shade];
    }
}
