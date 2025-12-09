#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144


typedef enum {
    FETCH_TILE_MAP,
    FETCH_TILE_DATA_LOW,
    FETCH_TILE_DATA_HIGH,
    FETCH_SLEEP,
    FETCH_PUSH
} FetcherState;

typedef struct {
    uint32_t color;
    uint8_t palette;
    bool priority;
    uint8_t index;
} Pixel;

typedef struct {
    Pixel q[16];
    int size;
    int head;
    int tail;
} PixelFIFO;


// Sprite struct
typedef struct {
    uint8_t y;
    uint8_t x;
    uint8_t tile_index;
    uint8_t flags;
    uint8_t index;
} Sprite;

typedef struct {
    Pixel pixels[8];
    uint8_t x_pos;
    uint8_t index;
    bool fetched;
} Sprite_FIFO_Entry;

typedef struct {
    Sprite entries[10];
    uint8_t count;
    Sprite_FIFO_Entry fetched_data[10]; 
} Sprite_List;


/* Struct for the PPU */
typedef struct {

    // LCD Registers
    uint8_t lcdc; // LCDControl.  All 8 bits do something, Note to self:     Check PanDocs for reference
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
    uint8_t mode; // Keeps track of the OAM mode
    uint8_t wly;  // Window line counter
    uint8_t wly_latch; // why
    // PPU timing
    int mode_cycles;
    int scanline;
    Sprite_List sprites;

    int line_ticks;
    
    // Pixel FIFO
    PixelFIFO bg_fifo;
    FetcherState fetcher_state;
    int fetcher_x;
    uint8_t tile_map_id;
    uint8_t tile_data_lo;
    uint8_t tile_data_hi;
    
    int lx;
    int pushed_x;
    int fetcher_cycles;
    bool window_visible;

    /*The framebuffer gets updated when the Scanlines are finshed.
      This happens when when the scanline reaches line 144. This is kept track of by the ly register.
      The PPU then triggers a VBlank interrupt, which causes the framebuffer to update*/
    uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
} PPU;



#endif