#include "cpu.h"

// Change Z
void set_Z(uint8_t result, Registers *cpu) {
    if (result == 0) {
        cpu->f |= FLAG_Z;
    } else {
        cpu->f &= ~FLAG_Z;
    }
}

// Change N
void set_N(bool condition, Registers *cpu) {
    if (condition)
        cpu->f |= FLAG_N;
    else
        cpu->f &= ~FLAG_N;
}

// Set H based on condition
void set_H(bool condition, Registers *cpu) {
    if (condition)
        cpu->f |= FLAG_H;
    else
        cpu->f &= ~FLAG_H;
}

// Set H for ADD
void set_H_add(uint8_t a, uint8_t b, Registers *cpu) {
    if (((a & 0x0F) + (b & 0x0F)) > 0x0F)
        cpu->f |= FLAG_H;
    else
        cpu->f &= ~FLAG_H;
}

// Set H for SUB    
void set_H_sub(uint8_t a, uint8_t b, Registers *cpu) {
    if ((a & 0x0F) < (b & 0x0F))
        cpu->f |= FLAG_H;
    else
        cpu->f &= ~FLAG_H;
}

// Set C based on condition
void set_C(bool condition, Registers *cpu) {
    if (condition)
        cpu->f |= FLAG_C;
    else
        cpu->f &= ~FLAG_C;
}

// Set C for ADD
void set_C_add(uint16_t result, Registers *cpu) {
    if (result > 0xFF)
        cpu->f |= FLAG_C;
    else
        cpu->f &= ~FLAG_C;
}

// Set C for SUB
void set_C_sub(uint8_t a, uint8_t b, Registers *cpu) {
    if (a < b)
        cpu->f |= FLAG_C;
    else
        cpu->f &= ~FLAG_C;
}
// -------------------------------------------------------------------------------------