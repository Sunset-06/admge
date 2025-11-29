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

void ppu_step(PPU *ppu, CPU *cpu) {
    uint32_t dots = cpu->cycles * 4;

    for (uint32_t i = 0; i < dots; i++) {
        ppu_tick(ppu, cpu);
    }

}

void ppu_tick(PPU *ppu, CPU *cpu) {
    ppu->mode_cycles++;

    uint8_t mode = ppu->stat & 0x03;

    switch (mode) {
    case 2: // OAM Scan - 80
        if (ppu->mode_cycles >= 80) {
            ppu->mode_cycles = 0;
            
            // Switch to Mode 3
            ppu->stat = (ppu->stat & 0xFC) | 0x03;
            
            // FIFO will go here
        }
        break;

    case 3: // Drawing - 172-289
        if (ppu->mode_cycles >= 172) {
            ppu->mode_cycles = 0;
            
            // Switch to Mode 0
            ppu->stat = (ppu->stat & 0xFC) | 0x00;
            
            render_scanline(ppu, cpu);
            if (ppu->stat & 0x08) {
                cpu->iflag |= 0x02;
            }
        }
        break;

    case 0: // H-Blank - Remaining time (total should be 456)
        if (ppu->mode_cycles >= 204) {
            ppu->mode_cycles = 0;
            ppu->ly++;

            // Check LYC coincidence
            if (ppu->ly == ppu->lyc) {
                ppu->stat |= 0x04; // Set LYC=LY
                if (ppu->stat & 0x40) cpu->iflag |= 0x02; // STAT
            } else {
                ppu->stat &= ~0x04;
            }

            if (ppu->ly == 144) {
                // Enter Mode 1 (V-Blank)
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

    case 1: // V-Blank (4560 dots total, 10 lines)
        if (ppu->mode_cycles >= 456) {
            ppu->mode_cycles = 0;
            ppu->ly++;

            if (ppu->ly > 153) {
                ppu->ly = 0;
                ppu->wly = 0;
                
                // Mode 2
                ppu->stat = (ppu->stat & 0xFC) | 0x02;
            }
        }
        break;
    }
}