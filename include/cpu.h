#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>


#define FLAG_Z 0x80
#define FLAG_N 0x40
#define FLAG_H 0x20
#define FLAG_C 0x10

// The registers a,f,b,c,d,s,h,l

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
    //stack pointer and program counter
    uint16_t sp;
    uint16_t pc;
} Registers;

extern void set_Z(uint8_t result, Registers *cpu);

extern void set_N(bool sub, Registers *cpu);

extern void set_H(bool condition, Registers *cpu);
extern void set_H_add(uint8_t a, uint8_t b, Registers *cpu);
extern void set_H_sub(uint8_t a, uint8_t b, Registers *cpu);
extern void set_H_add16(uint16_t a, uint16_t b, Registers *cpu);
extern void set_H_inc(uint8_t before, Registers *cpu);
extern void set_H_dec(uint8_t before, Registers *cpu);

extern void set_C(bool condition, Registers *cpu);
extern void set_C_add(uint8_t a, uint8_t b, Registers *cpu);
extern void set_C_sub(uint8_t a, uint8_t b, Registers *cpu);
extern void set_C_add16(uint16_t a, uint16_t b, Registers *cpu);
extern void set_C_sbc(uint8_t a, uint8_t b, bool carry, Registers *cpu);

extern void clear_flags(Registers *cpu);

#endif 