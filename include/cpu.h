#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "ppu.h"

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

/* So DIV is the main clock, it just accumulates CPU cycles, and most things use it for timing

*/
#define MEMORY_SIZE 0x10000

#define FLAG_Z 0x80
#define FLAG_N 0x40
#define FLAG_H 0x20
#define FLAG_C 0x10

// The registers a,f,b,c,d,e,h,l

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

    //timers
    uint16_t div;
    uint8_t tima, tma, tac;

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

extern void set_C(bool condition, CPU *cpu);
extern void set_C_add(uint8_t a, uint8_t b, CPU *cpu);
extern void set_C_sub(uint8_t a, uint8_t b, CPU *cpu);
extern void set_C_add16(uint16_t a, uint16_t b, CPU *cpu);
extern void set_C_sbc(uint8_t a, uint8_t b, bool carry, CPU *cpu);

extern void clear_flags(CPU *cpu);

// --------------------- memory functions
extern uint8_t read8(CPU *cpu, uint16_t addr);
extern void write8(CPU *cpu, uint16_t addr, uint8_t value);
extern uint16_t read16(CPU *cpu, uint16_t addr);
extern void write16(CPU *cpu, uint16_t addr, uint16_t value);

extern void stack_push(CPU *cpu, uint16_t value);
extern uint16_t stack_pop(CPU *cpu);

// --------------------- cpu functions
extern void start_cpu(CPU *cpu);

// --------------------- instructions
extern void run_inst(uint16_t opcode, CPU *cpu);
extern void run_pref_inst(CPU *cpu);
#endif 