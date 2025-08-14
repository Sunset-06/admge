#include "cpu.h"
#include "screen.h"
#include <string.h>

static const uint32_t GAMEBOY_COLORS[4] = {
    0xFFFFFFFF, // White
    0xFFAAAAAA, // Light Gray
    0xFF555555, // Dark Gray
    0xFF000000  // Black
};

void ppu_init(PPU *ppu) {
    memset(ppu->vram, 0, sizeof(ppu->vram));
    memset(ppu->oam, 0, sizeof(ppu->oam));

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

    // Reset timing
    ppu->mode_cycles = 0;
    ppu->scanline    = 0;

    // Clear framebuffer to white
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        ppu->framebuffer[i] = 0xFFFFFFFF; // ARGB: White
    }
}


void ppu_step(PPU *ppu, CPU *cpu, uint16_t cycles) {
    //PPU *ppu = cpu->ppu;
    ppu->mode_cycles += cycles;

    // Scanlines take 456 cycles
    if (ppu->mode_cycles >= 456) {
        ppu->mode_cycles -= 456;
        ppu->ly++;

        if (ppu->ly < 144) {    
            render_scanline(ppu);
            sdl_draw_scanline(ppu, ppu->ly);
        }

        if (ppu->ly == 144) {
            // VBlank triggered
            cpu->iflag |= 0x01; // Set VBlank interrupt
            sdl_present();
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

void render_scanline(PPU *ppu) {
    int row_start = ppu->ly * SCREEN_WIDTH;

    // Bit 3 of LCDC tells whichh tilemap base to use
    uint16_t tile_map_base = (ppu->lcdc & 0x08) ? 0x1C00 : 0x1800;

    // Bit 4 of LCDC tells which tiledata base address to use
    uint16_t tile_data_base;
    bool signed_indexing = false;
    if (ppu->lcdc & 0x10) {
        // Unsigned tile index, base 0x8000
        tile_data_base = 0x0000; 
    } else {
        // Signed tile index, base 0x8800
        tile_data_base = 0x0800; 
        signed_indexing = true; 
    }

    // Calculate which line of the BG we’re on
    uint8_t scrolled_y = (ppu->scy + ppu->ly) & 0xFF;
    uint8_t tile_row = scrolled_y / 8; 

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        uint8_t scrolled_x = (ppu->scx + x) & 0xFF;
        uint8_t tile_col = scrolled_x / 8; 

        uint16_t tile_index_addr = tile_map_base + (tile_row * 32) + tile_col;
        uint8_t tile_number = ppu->vram[tile_index_addr];

        // Adjust for signed indexing
        uint16_t tile_addr;
        if (signed_indexing) {
            int8_t signed_tile_number = (int8_t)tile_number;
            tile_addr = tile_data_base + ((signed_tile_number + 128) * 16);
        } else {
            tile_addr = tile_data_base + (tile_number * 16);
        }

        // Which line of the tile to fetch
        uint8_t tile_line = (scrolled_y % 8) * 2;
        uint8_t byte1 = ppu->vram[tile_addr + tile_line];
        uint8_t byte2 = ppu->vram[tile_addr + tile_line + 1];

        // Which bit in the byte (left pixel is bit 7)
        uint8_t bit = 7 - (scrolled_x % 8);
        uint8_t color_id = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);

        // Map the color ID through the BG palette
        uint8_t palette_shade = (ppu->bgp >> (color_id * 2)) & 0x03;

        // Store in framebuffer
        ppu->framebuffer[row_start + x] = GAMEBOY_COLORS[palette_shade];
        sdl_draw_scanline(ppu, ppu->ly);
    }
}

