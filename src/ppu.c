#include "cpu.h"
#include "peripherals.h"

static const uint32_t GAMEBOY_COLOURS[4] = {
    0xFFFFFFFF, // White
    0xFFAAAAAA, // Light Gray
    0xFF555555, // Dark Gray
    0xFF000000  // Black
};

void ppu_init(PPU *ppu) {
    // Default values for LCD registers 
    ppu->lcdc = 0x00; 
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
    ppu->line_ticks  = 0;
    ppu->scanline    = 0;

    // Clear framebuffer to white
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        ppu->framebuffer[i] = GAMEBOY_COLOURS[0];
    }
}

void lcd_off(PPU *ppu){
    ppu->ly = 0;
    // PPU enters Hblank
    ppu->stat = (ppu->stat & 0xFC) | 0x00;
    ppu->mode_cycles = 0;
    ppu->wly_latch = false;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        ppu->framebuffer[i] = GAMEBOY_COLOURS[0];
    }
    ppu->wly = 0;

}

void dma_transfer(CPU *cpu, uint8_t value) {
    //printf("Hit a DMA\n");
    uint16_t src = value << 8;
    memcpy(&cpu->memory[0xFE00], &cpu->memory[src], 0xA0);
}

//fifo helpers
void fifo_clear(PPU *ppu) {
    ppu->bg_fifo.size = 0;
    ppu->bg_fifo.head = 0;
    ppu->bg_fifo.tail = 0;
}

bool fifo_push(PPU *ppu, Pixel *pixels) {
    if (ppu->bg_fifo.size > 8) {
        return false; 
    }

    for (int i = 0; i < 8; i++) {
        ppu->bg_fifo.q[ppu->bg_fifo.tail] = pixels[i];
        ppu->bg_fifo.tail = (ppu->bg_fifo.tail + 1) % 16;
        ppu->bg_fifo.size++;
    }
    return true;
}

Pixel fifo_pop(PPU *ppu) {
    if (ppu->bg_fifo.size == 0) {
        Pixel empty = {0, 0, 0}; 
        return empty;
    }

    Pixel p = ppu->bg_fifo.q[ppu->bg_fifo.head];
    ppu->bg_fifo.head = (ppu->bg_fifo.head + 1) % 16;
    ppu->bg_fifo.size--;
    return p;
}

// In ppu.c

// This function must be called after the OAM scan (Mode 2) is complete, 
// typically right before or at the start of Mode 3.

void sort_sprites(PPU *ppu) {
    if (ppu->sprites.count <= 1) {
        return;
    }

    for (int i = 1; i < ppu->sprites.count; i++) {
        Sprite current_sprite = ppu->sprites.entries[i];
        Sprite_FIFO_Entry current_data = ppu->sprites.fetched_data[i];
        int j = i - 1;
        while (j >= 0) {
            Sprite check_sprite = ppu->sprites.entries[j];
            if (current_sprite.x < check_sprite.x) {
                ppu->sprites.entries[j + 1] = check_sprite;
                ppu->sprites.fetched_data[j + 1] = ppu->sprites.fetched_data[j];
                j--;
                continue;
            }
            //Tiebreaker
            if (current_sprite.x == check_sprite.x) {
                if (current_sprite.index < check_sprite.index) {
                    ppu->sprites.entries[j + 1] = check_sprite;
                    ppu->sprites.fetched_data[j + 1] = ppu->sprites.fetched_data[j];
                    j--;
                    continue;
                }
            }
            break; 
        }
        ppu->sprites.entries[j + 1] = current_sprite;
        ppu->sprites.fetched_data[j + 1] = current_data;
    }
}

void fetch_sprite_data(PPU *ppu, CPU *cpu, int sprite_index) {
    Sprite *obj = &ppu->sprites.entries[sprite_index];
    Sprite_FIFO_Entry *entry_data = &ppu->sprites.fetched_data[sprite_index];

    uint8_t obj_height = (ppu->lcdc & 0x04) ? 16 : 8;
    int row = ppu->ly + 16 - obj->y;
    
    if (obj->flags & 0x40) {
        row = obj_height - 1 - row;
    }
    uint16_t tile_addr = 0x8000 + (obj->tile_index * 16) + (row * 2);
    uint8_t data_lo = cpu->memory[tile_addr];
    uint8_t data_hi = cpu->memory[tile_addr + 1];

    for (int i = 0; i < 8; i++) {
        int bit = (obj->flags & 0x20) ? i : (7 - i); 
        
        uint8_t lo = (data_lo >> bit) & 1;
        uint8_t hi = (data_hi >> bit) & 1;
        uint8_t color_id = (hi << 1) | lo;

        entry_data->pixels[i].color = color_id;
        entry_data->pixels[i].palette = (obj->flags & 0x10) ? ppu->obp1 : ppu->obp0; 
        
        entry_data->pixels[i].priority = (obj->flags & 0x80); 
    }
    
    // Mark the data as fetched to prevent re-fetching on the same line
    entry_data->fetched = true;
}

void fetcher_step(PPU *ppu, CPU *cpu) {
    ppu->fetcher_cycles++;
    // takes 2 dots
    if (ppu->fetcher_cycles < 2) {
        return;
    }
    ppu->fetcher_cycles = 0;
    //bool window_visible = (ppu->lcdc & 0x20) && ppu->wly_latch && (ppu->wx < 166);

    switch (ppu->fetcher_state) {
        case FETCH_TILE_MAP:
            uint16_t map_base;
            uint8_t map_x;
            uint8_t map_y;
            
            if (ppu->window_visible) {
                // Draw the window here
                map_base = (ppu->lcdc & 0x40) ? 0x9C00 : 0x9800;
                
                map_x = ppu->fetcher_x; 
                map_y = ppu->wly;       
            } else {
                // Draw the bg here
                map_base = (ppu->lcdc & 0x08) ? 0x9C00 : 0x9800;
                map_x = (ppu->scx / 8 + ppu->fetcher_x) & 0x1F;
                map_y = (ppu->scy + ppu->ly) & 0xFF;
            }
            // tile id
            uint16_t map_addr = map_base + (map_y / 8 * 32) + map_x;
            ppu->tile_map_id = cpu->memory[map_addr];

            ppu->fetcher_state = FETCH_TILE_DATA_LOW;
            break;

        case FETCH_TILE_DATA_LOW:
            uint16_t data_base = (ppu->lcdc & 0x10) ? 0x8000 : 0x9000;
            uint16_t tile_addr;
            
            if (ppu->lcdc & 0x10)
                tile_addr = data_base + (ppu->tile_map_id * 16);
            else
                tile_addr = data_base + ((int8_t)ppu->tile_map_id * 16);

            uint8_t tile_row;
            if (ppu->window_visible) {
                tile_row = ppu->wly % 8;
            } else {
                tile_row = (ppu->scy + ppu->ly) % 8;
            }
            
            ppu->tile_data_lo = cpu->memory[tile_addr + (tile_row * 2)];
            ppu->fetcher_state = FETCH_TILE_DATA_HIGH;
            break;

        case FETCH_TILE_DATA_HIGH:
            uint16_t data_base_h = (ppu->lcdc & 0x10) ? 0x8000 : 0x9000;
            uint16_t tile_addr_h;

            if(ppu->lcdc & 0x10)
                tile_addr_h = data_base_h + (ppu->tile_map_id * 16);
            else
                tile_addr_h = data_base_h + ((int8_t)ppu->tile_map_id * 16);

            uint8_t tile_row_h;
            
            if (ppu->window_visible)
                tile_row_h = ppu->wly % 8;
            else
                tile_row_h = (ppu->scy + ppu->ly) % 8;
            
            ppu->tile_data_hi = cpu->memory[tile_addr_h + (tile_row_h * 2) + 1];
            
            ppu->fetcher_state = FETCH_PUSH; 
            break;

        case FETCH_PUSH:
            Pixel temp_pixels[8];
            for (int i = 0; i < 8; i++) {
                int bit = 7 - i; // MSB first
                uint8_t lo = (ppu->tile_data_lo >> bit) & 1;
                uint8_t hi = (ppu->tile_data_hi >> bit) & 1;
                temp_pixels[i].color = (hi << 1) | lo;
                temp_pixels[i].palette = ppu->bgp; // Store palette info
                temp_pixels[i].priority = false;
            }

            if (fifo_push(ppu, temp_pixels)) {
                ppu->fetcher_state = FETCH_TILE_MAP;
                ppu->fetcher_x++; 
            } 
            // else FIFO full, stall
            break;
            
        case FETCH_SLEEP:
            break;
    }
}

void ppu_step(PPU *ppu, CPU *cpu) {
    uint32_t dots = cpu->cycles * 4;

    for (uint32_t i = 0; i < dots; i++) {
        ppu_tick(ppu, cpu);
    }

}

void ppu_tick(PPU *ppu, CPU *cpu) {
    ppu->mode_cycles++;
    ppu->line_ticks++;
    uint8_t mode = ppu->stat & 0x03;

    switch (mode) {
    case 0: // HBlank - Remaining time (total should be 456)
        if (ppu->line_ticks >= 456) {
            ppu->mode_cycles = 0;
            ppu->line_ticks = 0;
            ppu->ly++;

            // Check LYC coincidence
            if (ppu->ly == ppu->lyc) {
                ppu->stat |= 0x04; // Set LYC=LY
                if (ppu->stat & 0x40) cpu->iflag |= 0x02; // STAT
            } else {
                ppu->stat &= ~0x04;
            }

            if (ppu->ly == 144) {
                // Enter Mode 1 (VBlank)
                ppu->stat = (ppu->stat & 0xFC) | 0x01;
                cpu->iflag |= 0x01; // Request VBlank
                
                // render
                present_screen(ppu);
            } else {
                // mode 2
                ppu->stat = (ppu->stat & 0xFC) | 0x02;
                
                // Handle OAM Interrupt
                if (ppu->stat & 0x20) {
                    cpu->iflag |= 0x02;
                }
            }
        }
        break;

    case 1: // VBlank (4560 dots total, 10 lines)
        if (ppu->line_ticks >= 456) {
            ppu->mode_cycles = 0;
            ppu->line_ticks = 0;
            ppu->ly++;

            if (ppu->ly > 153) {
                ppu->ly = 0;
                ppu->wly = 0;
                
                // Mode 2
                ppu->stat = (ppu->stat & 0xFC) | 0x02;
            }
        }
        break;
    case 2: // OAM Scan - 80
        if (ppu->mode_cycles == 1) { 
            ppu->sprites.count = 0;
            uint8_t obj_height = (ppu->lcdc & 0x04) ? 16 : 8;
            
            for (int i = 0; i < 40; i++) { 
                uint8_t sprite_y = cpu->memory[0xFE00 + (i * 4)];
                uint8_t sprite_x = cpu->memory[0xFE00 + (i * 4) + 1];
                
                if (ppu->ly + 16 >= sprite_y && ppu->ly + 16 < sprite_y + obj_height) {
                    
                    if (ppu->sprites.count < 10) { 
                        Sprite new_sprite;
                        new_sprite.y = sprite_y;
                        new_sprite.x = sprite_x;
                        new_sprite.tile_index = cpu->memory[0xFE00 + (i * 4) + 2];
                        new_sprite.flags = cpu->memory[0xFE00 + (i * 4) + 3];
                        
                        // LSB is ignored if 8x16
                        if (obj_height == 16) {
                            new_sprite.tile_index &= 0xFE; 
                        }
                        
                        ppu->sprites.entries[ppu->sprites.count] = new_sprite;
                        ppu->sprites.fetched_data[ppu->sprites.count].index = i;
                        ppu->sprites.fetched_data[ppu->sprites.count].x_pos = sprite_x;
                        ppu->sprites.fetched_data[ppu->sprites.count].fetched = false;
                        
                        ppu->sprites.count++;
                    }
                }
            }
        }

        
        if (ppu->line_ticks >= 80) {
            ppu->mode_cycles = 0;
            sort_sprites(ppu);
            // Switch to Mode 3
            ppu->stat = (ppu->stat & 0xFC) | 0x03;
            
            ppu->fetcher_x = 0;
            ppu->pushed_x = 0;
            ppu->lx = 0;
            ppu->fetcher_state = FETCH_TILE_MAP;
            ppu->fetcher_cycles = 0;
            fifo_clear(ppu);
        }
        
        break;

    case 3: // Drawing - 172-289
        fetcher_step(ppu, cpu);

        if ((ppu->lcdc & 0x20) && (ppu->ly >= ppu->wy) && (ppu->lx + 7 == ppu->wx)) {
            ppu->window_visible = true;
            ppu->fetcher_state = FETCH_TILE_MAP;
            ppu->fetcher_x = 0; 
            fifo_clear(ppu);
        }
        for (int i = 0; i < ppu->sprites.count; i++) {
            Sprite_FIFO_Entry *entry_data = &ppu->sprites.fetched_data[i];
            if (!entry_data->fetched && (entry_data->x_pos / 8) == ppu->fetcher_x) {
                fetch_sprite_data(ppu, cpu, i);
            }
        }

        
        if (ppu->bg_fifo.size > 0) {
            Pixel bg_p = fifo_pop(ppu);
            Pixel final_p = bg_p;
            
            if (ppu->lcdc & 0x02) {
                uint8_t winning_x_pos = 168;
                uint8_t winning_oam_index = 40;

                // for all sprites
                for (int i = 0; i < ppu->sprites.count; i++) {
                    Sprite_FIFO_Entry *obj_data = &ppu->sprites.fetched_data[i];
                    uint8_t screen_x = ppu->pushed_x; 

                    if (obj_data->x_pos > 0 && screen_x + 8 >= obj_data->x_pos && screen_x + 8 < obj_data->x_pos + 8) {
                    
                        int sprite_pixel_offset = screen_x + 8 - obj_data->x_pos; 
                        Pixel obj_p = obj_data->pixels[sprite_pixel_offset];
                        
                        if (obj_p.color > 0) {
                            
                            if (final_p.palette == ppu->bgp) {
                                bool bg_priority = obj_p.priority;
                                
                                if (!(bg_priority && bg_p.color != 0)) {
                                    final_p = obj_p;
                                    winning_x_pos = obj_data->x_pos;
                                    winning_oam_index = obj_data->index;
                                }
                            
                            } else {
                                if (obj_data->x_pos < winning_x_pos || 
                                (obj_data->x_pos == winning_x_pos && obj_data->index < winning_oam_index)) {
                                    final_p = obj_p;
                                    winning_x_pos = obj_data->x_pos;
                                    winning_oam_index = obj_data->index;
                                }
                            }
                        }
                    }
                
                }
            }

            if (ppu->lx >= (ppu->scx % 8)) {
        
                uint8_t color_id = final_p.color;
                uint8_t shade;

                // Determine shade based on the palette stored in the final_p pixel
                shade = (final_p.palette >> (color_id * 2)) & 0x03;
                
                // Write to framebuffer
                int fb_idx = ppu->ly * SCREEN_WIDTH + ppu->pushed_x;
                ppu->framebuffer[fb_idx] = GAMEBOY_COLOURS[shade];
                
                ppu->pushed_x++;
            }
            
            ppu->lx++;
        }

        if (ppu->pushed_x >= 160) {
            ppu->stat = (ppu->stat & 0xFC) | 0x00; // Switch to HBlank
            if (ppu->window_visible) {
                ppu->wly++;
            }
            
            // Check for HBlank
            if (ppu->stat & 0x08) {
                cpu->iflag |= 0x02;
            }
        }
        break;
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
        case 0xFF40:ppu->lcdc = value; 
                    // if lcd is being switched off, set window line counter back to 0
                    if (!(value & 0x80))// Bit 7 is the LCD enable bit
                        lcd_off(ppu);
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