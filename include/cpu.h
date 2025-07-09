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

#endif 