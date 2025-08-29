#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

/* A Note about the Clock and Cycles
    Every op will take a certain amount of m-cycles
        For Example: 0x01 LD r16, u16 has:
            fetch
            read
            read
    However evry m-cycle takes 4 of actual clock cycles, abstacted away.
    So that means that LD r16, u16 has:
        3 m-cycles
        12 clock cycles

    The clock cycles are what the PPU will step based on.
    So the 'accurate-enough' approach is counting the m-cycles and multiplying by 4.
    This is what i'll be doing.
*/

/* 
    So DIV is the main clock, it just accumulates CPU cycles, and most things use it for timing
*/

#define MEMORY_SIZE 0x10000
#define BOOTROM_SIZE 0x100

#define FLAG_Z 0x80
#define FLAG_N 0x40
#define FLAG_H 0x20
#define FLAG_C 0x10

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

#define VRAM_SIZE 0x2000  // 8KB
#define OAM_SIZE 0xA0     // 160 bytes

// Sprite struct
typedef struct {
    uint8_t y;
    uint8_t x;
    uint8_t tile_index;
    uint8_t flags;
} Sprite;


/* Struct for the PPU */
typedef struct {

    // LCD Registers
    uint8_t lcdc; // LCDControl.  All 8 bits do something, Note to self: Check PanDocs for reference
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

    /*The framebuffer gets updated when the Scanlines are finshed.
      This happens when when the scanline reaches line 144. This is kept track of by the ly register.
      The PPU then triggers a VBlank interrupt, which causes the framebuffer to update*/
    uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
} PPU;


/* Struct for the Registers a,f,b,c,d,e,h,l */
typedef struct {
    union {
        struct {
            uint8_t f, a;
        };
        uint16_t af;
    };
    union {
        struct { 
            uint8_t c, b; 
        };
        uint16_t bc;
    };
    union {
        struct { 
            uint8_t e, d; 
        };
        uint16_t de;
    };
    union {
        struct { 
            uint8_t l, h; 
        };
        uint16_t hl;
    };
} Registers;


/* The main CPU struct */
typedef struct {
    Registers regs;
    PPU ppu;

    //stack pointer and program counter
    uint16_t sp;
    uint16_t pc;

    uint8_t memory[MEMORY_SIZE];

    bool ime;
    uint8_t ie;  // Interrupt Enable
    uint8_t iflag; // Interrupt Flag 

    bool halted;
    bool stopped;
    bool ime_enable;
    bool bootrom_flag;
    uint8_t bootrom[BOOTROM_SIZE];

    //timers
    uint16_t div;
    uint8_t tima, tma, tac;
    int timer_counter;

    // storing the state of all 8 buttons together
    // St Sl B A - upper nibble is buttons
    // D U L R   - lower nibble is DPAD
    uint8_t joypad;

    uint64_t cycles;
} CPU;

// --------------------- flag functions
extern void set_Z(uint8_t result, CPU *cpu);

extern void set_N(bool sub, CPU *cpu);

extern void set_H(bool condition, CPU *cpu);
extern void set_H_add(uint8_t a, uint8_t b, CPU *cpu);
extern void set_H_sub(uint8_t a, uint8_t b, CPU *cpu);
extern void set_H_add16(uint16_t a, uint16_t b, CPU *cpu);
extern void set_H_inc(uint8_t before, CPU *cpu);
extern void set_H_dec(uint8_t before, CPU *cpu);
extern void set_H_adc(uint8_t a, uint8_t b, uint8_t c, CPU *cpu);
extern void set_H_sbc(uint8_t a, uint8_t b, uint8_t c, CPU *cpu);

extern void set_C(bool condition, CPU *cpu);
extern void set_C_add(uint8_t a, uint8_t b, CPU *cpu);
extern void set_C_sub(uint8_t a, uint8_t b, CPU *cpu);
extern void set_C_add16(uint16_t a, uint16_t b, CPU *cpu);
extern void set_C_adc(uint16_t res, CPU *cpu);
extern void set_C_sbc(uint16_t res, CPU *cpu);

extern void clear_flags(CPU *cpu);

// --------------------- memory bus functions
extern uint8_t read8(CPU *cpu, uint16_t addr);
extern void write8(CPU *cpu, uint16_t addr, uint8_t value);
extern uint16_t read16(CPU *cpu, uint16_t addr);
extern void write16(CPU *cpu, uint16_t addr, uint16_t value);

extern void stack_push(CPU *cpu, uint16_t value);
extern uint16_t stack_pop(CPU *cpu);

// --------------------- cpu functions
extern void start_cpu(CPU *cpu);
extern void start_cpu_noboot(CPU *cpu);
extern void cpu_step(CPU *cpu);

// --------------------- instructions
extern void run_inst(uint8_t opcode, CPU *cpu);
extern void run_pref_inst(CPU *cpu, uint8_t opcode);

// --------------------- ppu functions

extern void ppu_init(PPU *ppu);
extern void ppu_step(PPU *ppu, CPU *cpu);
extern uint8_t ppu_read(CPU *cpu, uint16_t addr);
extern void ppu_write(CPU *cpu, uint16_t addr, uint8_t value);
extern void render_scanline(PPU *ppu, CPU *cpu);

#endif 