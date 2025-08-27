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

void dma_transfer(CPU *cpu, uint8_t value) {
    uint16_t src = value << 8;
    for (int i = 0; i < 0xA0; i++) {
        write8(cpu, 0xFE00 + i, read8(cpu, src + i));
    }
    cpu->ppu.mode_cycles += 160*4;
}


void ppu_step(PPU *ppu, CPU *cpu) {
    ppu->mode_cycles += cpu->cycles*4;
    // Each scanline takes 456 cycles

    //OAM
    switch (ppu->stat & 0x03) {
    case 0: // H-Blank
        if (ppu->mode_cycles >= 204) {
            ppu->mode_cycles -= 204;
            ppu->ly++;

            if (ppu->ly == 144) {
                // Enter V-Blank
                ppu->stat = (ppu->stat & 0xFC) | 0x01;
                cpu->iflag |= 0x01; // Request V-Blank Interrupt
                sdl_present(ppu);
            } else {
                // Enter OAM Scan for the next scanline
                ppu->stat = (ppu->stat & 0xFC) | 0x02;
            }
        }
        break;

    case 1: // V-Blank
        if(ppu->mode_cycles >= 456) {
            ppu->mode_cycles -= 456;
            ppu->ly++;

            if (ppu->ly > 153) {
                ppu->ly = 0;
                // Return to OAM Scan for the first scanline of the new frame
                ppu->stat = (ppu->stat & 0xFC) | 0x02;
            }
        }
        break;

    case 2: // OAM Scan
        if (ppu->mode_cycles >= 80) {
            ppu->mode_cycles -= 80;
            // Enter Drawing mode
            ppu->stat = (ppu->stat & 0xFC) | 0x03;
        }
        break;

    case 3: // Drawing
        if (ppu->mode_cycles >= 172) {
            ppu->mode_cycles -= 172;
            // Enter H-Blank
            ppu->stat = (ppu->stat & 0xFC) | 0x00;
            render_scanline(ppu, cpu);
        }
        break;
    }

    if (ppu->ly == ppu->lyc) {
        ppu->stat |= 0x04; // Set coincidence flag
        if (ppu->stat & 0x40) { // If LYC=LY interrupt is enabled
            cpu->iflag |= 0x02; // Request STAT interrupt
        }
    } else {
        ppu->stat &= ~0x04; // Clear coincidence flag
    }
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
        case 0xFF44: ppu->ly   = 0;     break; // <- This one is read only
        case 0xFF45: ppu->lyc  = value; break;
        case 0xFF47: ppu->bgp  = value; break;
        case 0xFF48: ppu->obp0 = value; break;
        case 0xFF49: ppu->obp1 = value; break;
        case 0xFF4A: ppu->wy   = value; break;
        case 0xFF4B: ppu->wx   = value; break;

        case 0xFF46: // DMA Trigger
            dma_transfer(cpu, value);
            break;
    }
    ////printf("\n\n\n VRAM:");
    for(int i=0x8000; i<0x9FFF;i++){
        ////printf("%02x ", cpu->memory[i]);
    }
}

void render_scanline(PPU *ppu, CPU *cpu) {
    if (!(ppu->lcdc & 0x80)) return; // LCD disabled

    // Determine base addresses from LCDC register
    uint16_t bg_tile_map_base = (ppu->lcdc & 0x08) ? 0x9C00 : 0x9800;
    uint16_t window_tile_map_base = (ppu->lcdc & 0x40) ? 0x9C00 : 0x9800;
    uint16_t tile_data_base = (ppu->lcdc & 0x10) ? 0x8000 : 0x9000;

    // Check if the window is enabled and visible on this scanline
    bool window_enabled = (ppu->lcdc & 0x20) && (ppu->ly >= ppu->wy);
    uint8_t window_y = ppu->ly - ppu->wy; 

    // Rendering BG and Windoew
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        // Check if the current pixel is inside the window's X range
        bool in_window = window_enabled && (x >= (ppu->wx - 7));

        if (in_window) {
            uint8_t window_x = x - (ppu->wx - 7); 

            uint16_t tile_map_addr = window_tile_map_base + ((window_y / 8) * 32) + (window_x / 8);
            uint8_t tile_number = cpu->memory[tile_map_addr];

            uint16_t tile_addr;
            // Choose signed or unsigned addressing
            if (tile_data_base == 0x8000) {
                tile_addr = tile_data_base + (tile_number * 16);
            } else {
                tile_addr = tile_data_base + (((int8_t)tile_number + 128) * 16);
            }

            uint8_t line_in_tile = window_y % 8;
            uint8_t byte1 = cpu->memory[tile_addr + line_in_tile * 2];
            uint8_t byte2 = cpu->memory[tile_addr + line_in_tile * 2 + 1];

            // Combine the bytes to get the color ID
            uint8_t bit = 7 - (window_x % 8);
            uint8_t color_id = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);
            
            // Get the actual color from the palette and write to the framebuffer
            uint8_t shade = (ppu->bgp >> (color_id * 2)) & 0x03;
            ppu->framebuffer[ppu->ly * SCREEN_WIDTH + x] = GAMEBOY_COLORS[shade];

        } else if (ppu->lcdc & 0x01) {
            // Draw Background Pixel 
            uint8_t scrolled_y = (ppu->scy + ppu->ly) % 256; 
            uint8_t scrolled_x = (ppu->scx + x) % 256;
            
            // Get tile from the background tile map
            uint16_t tile_index_addr = bg_tile_map_base + ((scrolled_y / 8) * 32) + (scrolled_x / 8);
            uint8_t tile_number = cpu->memory[tile_index_addr];
            
            // Calculate the address of the tile's pixel data
            uint16_t tile_addr;
            if (tile_data_base == 0x8000) {
                 tile_addr = tile_data_base + (tile_number * 16);
            } else {
                 tile_addr = tile_data_base + (((int8_t)tile_number + 128) * 16);
            }
            
            // Get the two bytes for the current line of the tile
            uint8_t tile_line = scrolled_y % 8;
            uint8_t byte1 = cpu->memory[tile_addr + tile_line * 2];
            uint8_t byte2 = cpu->memory[tile_addr + tile_line * 2 + 1];
            
            // Combine the bytes to get the color ID
            uint8_t bit = 7 - (scrolled_x % 8);
            uint8_t color_id = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);

            // Get the actual color from the palette and write to the framebuffer
            uint8_t shade = (ppu->bgp >> (color_id * 2)) & 0x03;
            ppu->framebuffer[ppu->ly * SCREEN_WIDTH + x] = GAMEBOY_COLORS[shade];
        }
    }

    // OAM Sprites 
    if (!(ppu->lcdc & 0x02)) return; // Sprites disabled

    uint8_t sprite_height = (ppu->lcdc & 0x04) ? 16 : 8;
    Sprite oam[40];
    memcpy(oam, &cpu->memory[0xFE00], sizeof(oam));

    Sprite visible_sprites[10];
    int sprite_count = 0;

    // Limiting to 10 sprites per scanline
    for (int i = 0; i < 40 && sprite_count < 10; i++) {
        int sprite_y = oam[i].y - 16;
        if (ppu->ly >= sprite_y && ppu->ly < (sprite_y + sprite_height)) {
            visible_sprites[sprite_count++] = oam[i];
        }
    }

    // reverse OAM order of priority
    for (int i = sprite_count - 1; i >= 0; i--) {
        Sprite s = visible_sprites[i];
        int sprite_x = s.x - 8;

        // Sprite attributes
        bool x_flip = (s.attributes & 0x20);
        bool y_flip = (s.attributes & 0x40);
        bool bg_priority = (s.attributes & 0x80);
        uint8_t palette_number = (s.attributes & 0x10) ? ppu->obp1 : ppu->obp0;

        uint8_t line_in_sprite = ppu->ly - (s.y - 16);
        if (y_flip) {
            line_in_sprite = (sprite_height - 1) - line_in_sprite;
        }

        uint16_t tile_addr = 0x8000 + (s.tile_index * 16);
        uint8_t byte1 = cpu->memory[tile_addr + line_in_sprite * 2];
        uint8_t byte2 = cpu->memory[tile_addr + line_in_sprite * 2 + 1];

        for (int x_pixel = 0; x_pixel < 8; x_pixel++) {
            int pixel_x = sprite_x + x_pixel;
            if (pixel_x < 0 || pixel_x >= SCREEN_WIDTH) continue;

            uint8_t bit = x_flip ? x_pixel : (7 - x_pixel);
            uint8_t color_id = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);

            // color 0 = transparent
            if (color_id == 0) continue;

            // Handle background priority: sprite is behind BG if bg_priority is set
            // and the background/window pixel is not color 0.
            // Note: This check requires you to have stored the BG color ID, not the final color.
            // A simpler, though less accurate, check is shown here.
            uint32_t bg_pixel_color = ppu->framebuffer[ppu->ly * SCREEN_WIDTH + pixel_x];
            if (bg_priority && bg_pixel_color != GAMEBOY_COLORS[0]) {
                 continue;
            }

            uint8_t shade = (palette_number >> (color_id * 2)) & 0x03;
            ppu->framebuffer[ppu->ly * SCREEN_WIDTH + pixel_x] = GAMEBOY_COLORS[shade];
        }
    }
}
