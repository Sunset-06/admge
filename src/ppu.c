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

void fetcher_step(PPU *ppu, CPU *cpu) {
    ppu->fetcher_cycles++;
    // takes 2 dots
    if (ppu->fetcher_cycles < 2) {
        return;
    }
    ppu->fetcher_cycles = 0;

    switch (ppu->fetcher_state) {
        case FETCH_TILE_MAP:
            uint16_t map_base = (ppu->lcdc & 0x08) ? 0x9C00 : 0x9800;
            
            uint8_t map_x = (ppu->scx / 8 + ppu->fetcher_x) & 0x1F;
            uint8_t map_y = (ppu->scy + ppu->ly) & 0xFF;
            
            // tile id
            uint16_t map_addr = map_base + (map_y / 8 * 32) + map_x;
            ppu->tile_map_id = cpu->memory[map_addr];

            ppu->fetcher_state = FETCH_TILE_DATA_LOW;
            break;

        case FETCH_TILE_DATA_LOW:
            uint16_t data_base = (ppu->lcdc & 0x10) ? 0x8000 : 0x9000;
            uint16_t tile_addr;
            
            if (ppu->lcdc & 0x10) {
                tile_addr = data_base + (ppu->tile_map_id * 16);
            } else {
                tile_addr = data_base + ((int8_t)ppu->tile_map_id * 16);
            }

            uint8_t tile_row = (ppu->scy + ppu->ly) % 8;
            
            ppu->tile_data_lo = cpu->memory[tile_addr + (tile_row * 2)];
            ppu->fetcher_state = FETCH_TILE_DATA_HIGH;
            break;

        case FETCH_TILE_DATA_HIGH:
            uint16_t data_base_h = (ppu->lcdc & 0x10) ? 0x8000 : 0x9000;
            uint16_t tile_addr_h;
            if (ppu->lcdc & 0x10) tile_addr_h = data_base_h + (ppu->tile_map_id * 16);
            else tile_addr_h = data_base_h + ((int8_t)ppu->tile_map_id * 16);

            uint8_t tile_row_h = (ppu->scy + ppu->ly) % 8;
            
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
            } else {
                // FIFO full, stall
            }
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
        if (ppu->line_ticks >= 80) {
            ppu->mode_cycles = 0;
            
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

        if (ppu->bg_fifo.size > 0) {
            
            Pixel p = fifo_pop(ppu);

            // If SCX % 8 != 0, we need to discard the first few pixels of the first tile.
            if (ppu->lx >= (ppu->scx % 8)) {
                
                // get colour from Palette
                uint8_t color_id = p.color;
                uint8_t shade = (p.palette >> (color_id * 2)) & 0x03;
                
                // Write to framebuffer
                int fb_idx = ppu->ly * SCREEN_WIDTH + ppu->pushed_x;
                ppu->framebuffer[fb_idx] = GAMEBOY_COLOURS[shade];
                
                ppu->pushed_x++;
            }
            
            ppu->lx++;
        }

        // 3. Check for End of Scanline
        if (ppu->pushed_x >= 160) {
            ppu->stat = (ppu->stat & 0xFC) | 0x00; // Switch to HBlank
            
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