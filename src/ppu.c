#include "cpu.h"
#include "screen.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
static const uint32_t GAMEBOY_COLOURS[4] = {
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
    ppu->wly  = 0x00;
    ppu->wly_latch = false;

    ppu->mode_cycles = 0;
    ppu->scanline    = 0;

    // Clear framebuffer to white
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        ppu->framebuffer[i] = GAMEBOY_COLOURS[0];
    }
}

void ppu_step(PPU *ppu, CPU *cpu) {
    ppu->mode_cycles += cpu->cycles*4;

    //OAM
    switch (ppu->stat & 0x03) {
    case 0: // HBlank
        //printf("Enter HBlank\n");
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

        if (ppu->ly == ppu->lyc) {
            ppu->stat |= 0x04; // Set coincidence flag
            if (ppu->stat & 0x40) { // If LYC=LY interrupt is enabled
                cpu->iflag |= 0x02; // Request STAT interrupt
            }
        } else {
            ppu->stat &= ~0x04; // Clear coincidence flag
        }
        break;

    case 1: // VBlank
        //printf("Enter VBlank\n");
        if(ppu->mode_cycles >= 456) {
            ppu->mode_cycles -= 456;
            ppu->ly++;
            if (ppu->ly > 153) {
                ppu->ly = 0;
                ppu->wly = 0;
                ppu->wly_latch = false;
                // Frame finished, now set mode back to OAM Scan
                ppu->stat = (ppu->stat & 0xFC) | 0x02;
            }
        }
        break;

    case 2: // OAM Scan
        //printf("Enter OAM scan\n");
        if (ppu->mode_cycles >= 80) {
            ppu->mode_cycles -= 80;
            // Enter Drawing mode
            ppu->stat = (ppu->stat & 0xFC) | 0x03;
        }
        break;

    case 3: // Drawing
        //printf("Drawing :D\n\n");
        if (ppu->mode_cycles >= 172) {
            ppu->mode_cycles -= 172;
            // Enter H-Blank
            ppu->stat = (ppu->stat & 0xFC) | 0x00;
            render_scanline(ppu, cpu);
        }
        break;
    }
}

void dma_transfer(CPU *cpu, uint8_t value) {
    uint16_t src = value << 8;
    memcpy(&cpu->memory[0xFE00], &cpu->memory[src], 0xA0);
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
        case 0xFF40:ppu->lcdc = value; 
                    // if lcd is being switched off, set window line counter back to 0
                    if (!(value & 0x80))// Bit 7 is the LCD enable bit
                        ppu->wly = 0; 
                    break;
        case 0xFF41: ppu->stat = value; break;
        case 0xFF42: ppu->scy  = value; break;
        case 0xFF43: ppu->scx  = value; break;
        case 0xFF44:ppu->ly = 0;     
                    break; 
        case 0xFF45: ppu->lyc  = value; break;
        case 0xFF47: ppu->bgp  = value; break;
        case 0xFF48: ppu->obp0 = value; break;
        case 0xFF49: ppu->obp1 = value; break;
        case 0xFF4A: ppu->wy   = value; break;
        case 0xFF4B: ppu->wx   = value; break;

        case 0xFF46: // DMA Trigger
            dma_transfer(cpu, value);
            cpu->cycles += 160;
            break;
    }
}

/*IMPORTANT --------------------------------------------------------------------------
    "window is active" also probably needs a tweak - there's a hidden "window y latch" that is set to false at the start of the 
    frame and true when WY==LY the first time in the frame.
    The window will then draw on all lines after that, if WX is on-screen
    i.e. the condition for window is "WY_latch is true and WX is <=166" not LY>=WY 
*/

// It says bg on the tin, but this renders both bg and window
void render_bg(PPU *ppu, CPU *cpu){
    if(ppu->ly == ppu->wy)
        ppu->wly_latch = true;
    // LCDC sets these values
    //get the addressing mode: $8000 unsigned or $9000 signed (8800 to 97FF)
    uint16_t tile_data_base = (ppu->lcdc & 0x10) ? 0x8000 : 0x9000;
    // 0 = 9800–9BFF; 1 = 9C00–9FFF
    uint16_t bg_tile_map_base = (ppu->lcdc & 0x08) ? 0x9C00 : 0x9800;
    /// 0 = 9800–9BFF; 1 = 9C00–9FFF
    uint16_t window_tile_map_base = (ppu->lcdc & 0x40) ? 0x9C00 : 0x9800;

    // Check if the window is enabled and visible on this scanline
    bool window_visible = (ppu->lcdc & 0x20) && ppu->wly_latch && (ppu->wx < 166);

    // Rendering BG and Window
    // for every pixel in this scanline
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        // Check if the current pixel is inside the window's X range
        bool in_window = window_visible && (i >= (ppu->wx - 7));
        if (in_window) {   
            uint8_t window_x = i - (ppu->wx - 7);  
            
            //8 bits per tile and 
            uint16_t tile_map_addr = window_tile_map_base  + ((ppu->wly / 8) * 32) + (window_x / 8);
            uint8_t tile_number = cpu->memory[tile_map_addr];
            
            uint16_t tile_addr;
            // Choose signed or unsigned addressing
            if (tile_data_base == 0x8000) {
                tile_addr = tile_data_base + (tile_number * 16);
            } else {
                tile_addr = tile_data_base + ((int8_t)tile_number * 16);
            }

            uint8_t line_in_tile = ppu->wly % 8;
            uint8_t byte1 = cpu->memory[tile_addr + line_in_tile * 2];
            uint8_t byte2 = cpu->memory[tile_addr + line_in_tile * 2 + 1];
 
            // Combine the bytes to get the colour ID
            uint8_t bit = 7 - (window_x % 8);
            uint8_t colour_id = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);

            // Get the actual colour from the palette and write to the framebuffer
            uint8_t shade = (ppu->bgp >> (colour_id * 2)) & 0x03;
            ppu->framebuffer[ppu->ly * SCREEN_WIDTH + i] = GAMEBOY_COLOURS[shade];

    // ------------------------------------------------------------------------------------        

        } else if (ppu->lcdc & 0x01) {
            // Draw Background Pixel 
            uint8_t scrolled_y = (ppu->scy + ppu->ly) % 256; 
            uint8_t scrolled_x = (ppu->scx + i) % 256;
            
            // Get tile from the background tile map
            uint16_t tile_index_addr = bg_tile_map_base + ((scrolled_y / 8) * 32) + (scrolled_x / 8);
            uint8_t tile_number = cpu->memory[tile_index_addr];
            
            // Calculate the address of the tile's pixel data
            uint16_t tile_addr;
            if (tile_data_base == 0x8000) {
                 tile_addr = tile_data_base + (tile_number * 16);
            } else {
                 tile_addr = tile_data_base + ((int8_t)tile_number * 16);
            }
            
            // Get the two bytes for the current line of the tile
            uint8_t tile_line = scrolled_y % 8;
            uint8_t byte1 = cpu->memory[tile_addr + tile_line * 2];
            uint8_t byte2 = cpu->memory[tile_addr + tile_line * 2 + 1];
            
            // Combine the bytes to get the colour ID
            uint8_t bit = 7 - (scrolled_x % 8);
            uint8_t colour_id = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);

            // Get the actual colour from the palette and write to the framebuffer
            uint8_t shade = (ppu->bgp >> (colour_id * 2)) & 0x03;
            ppu->framebuffer[ppu->ly * SCREEN_WIDTH + i] = GAMEBOY_COLOURS[shade];
        }
    }
    // Incrementing window line counter
    // If the window exists in this scanline
    if (window_visible) {
        ppu->wly += 1;
    }
}

// Insertion sorts the objects based on x coordinate order 
void ins_sort_obj(Sprite arr[], int n) {
    int i, j;
    Sprite key;
    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i - 1;
        while (j >= 0 && arr[j].x > key.x) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}


void render_objects(PPU *ppu, CPU *cpu){

    if (!(ppu->lcdc & 0x02)) return; // Sprites disabled
    uint8_t obj_height = (ppu->lcdc & 0x04) ? 16 : 8;
    //oam is storing 40 obects. each object has the four properties of the Sprite struct
    Sprite oam[40];
    memcpy(oam, &cpu->memory[0xFE00], 0xA0);

    // This buffer stores the visible objects for current scanline
    Sprite sp_buffer[10];
    int obj_count = 0;

    /*
    According to GBEDG:
        Sprite X-Position must be greater than 0
        LY + 16 must be greater than or equal to Sprite Y-Position
        LY + 16 must be less than Sprite Y-Position + Sprite Height (8 in Normal Mode, 16 in Tall-Sprite-Mode)
        The amount of sprites already stored in the OAM Buffer must be less than 10
    
    Adding the required objects to sp_buffer
    This is esentially OAM scan for the current scanline
    */
    for (int i = 0; i < 40 && obj_count < 10; i++) {
        if(oam[i].x <= 0) continue;
        if(ppu->ly+16 < oam[i].y) continue;
        if(ppu->ly + 16 >= oam[i].y + obj_height) continue;
        sp_buffer[obj_count++] = oam[i];
    }

    // Sort the objects by x coordinate increasing priority
    ins_sort_obj(sp_buffer, obj_count);
    //printf("%u objects in scanline no: %u\n", obj_count, ppu->ly);
    
    for(int i=obj_count-1; i>=0; i--){
        Sprite curr_sprite = sp_buffer[i];
        
        // select the obp and flip from flags
        uint8_t palette_num = (curr_sprite.flags & 0x10) ? 1 : 0;
        uint8_t palette = palette_num ? ppu->obp1 : ppu->obp0;
        bool y_flip = (curr_sprite.flags & 0x40);
        bool x_flip = (curr_sprite.flags & 0x20);
        bool priority = (curr_sprite.flags & 0x80);

        uint8_t tile_y = (ppu->ly - curr_sprite.y + 16);

        if (y_flip) {
            tile_y = obj_height - 1 - tile_y;
        }

        uint8_t tile_index = curr_sprite.tile_index;
        if (obj_height == 16) {
            if (tile_y < 8) {
                tile_index &= 0xFE; // Use first tile   
            } else {
                tile_index |= 0x01; // Use second tile
                tile_y -= 8;
            }
        }
        uint16_t tile_addr = 0x8000 + (tile_index * 16) + (tile_y * 2);

        // This is pixel data (2bpp)
        uint8_t bytelo = cpu->memory[tile_addr];
        uint8_t bytehi = cpu->memory[tile_addr + 1];

        for(int j=0;  j<8; j++){
            int pixel_index = x_flip ? 7 - j : j;  
            // we need msb to get the colours     
            uint8_t colour_bit1 = (bytelo >> (7 - pixel_index)) & 1;
            uint8_t colour_bit2 = (bytehi >> (7 - pixel_index)) & 1;
            uint8_t colour_id = (colour_bit2 << 1) | colour_bit1;

            if (colour_id == 0) continue;

            int screen_x = curr_sprite.x - 8 + j;
            
            if (screen_x < 0 || screen_x >= 160) continue;
            // --------------------------------------------------------

            uint32_t index = ppu->ly * SCREEN_WIDTH + screen_x;
            uint8_t shade_index = (palette >> (colour_id * 2)) & 0x03;
            if (priority && ppu->framebuffer[index] != GAMEBOY_COLOURS[0]) continue;
            ppu->framebuffer[index] = GAMEBOY_COLOURS[shade_index];

        }
    }
}

void render_scanline(PPU *ppu, CPU *cpu) {
    if (!(ppu->lcdc & 0x80)) return; // LCD disabled
    render_bg(ppu, cpu);
    render_objects(ppu, cpu);
}
