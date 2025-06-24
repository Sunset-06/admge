#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

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