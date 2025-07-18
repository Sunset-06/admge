#include "cpu.h"

// Change Z based on result <- NOTE this is the exact opposite of all other flag functions
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

void set_H_add16(uint16_t a, uint16_t b, Registers *cpu) {
    if (((a & 0x0FFF) + (b & 0x0FFF)) > 0x0FFF)
        cpu->f |= FLAG_H;
    else
        cpu->f &= ~FLAG_H;
}

void set_H_inc(uint8_t before, Registers *cpu) {
    if (((before & 0x0F) + 1) > 0x0F)
        cpu->f |= FLAG_H;
    else
        cpu->f &= ~FLAG_H;
}

void set_H_dec(uint8_t before, Registers *cpu) {
    if ((before & 0x0F) == 0)
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
void set_C_add(uint8_t a, uint8_t b, Registers *cpu) {
    if ((uint16_t)a + (uint16_t)b > 0xFF)
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

void set_C_add16(uint16_t a, uint16_t b, Registers *cpu) {
    if ((uint32_t)a + (uint32_t)b > 0xFFFF)
        cpu->f |= FLAG_C;
    else
        cpu->f &= ~FLAG_C;
}

void set_C_sbc(uint8_t a, uint8_t b, bool carry, Registers *cpu) {
    uint16_t temp = a - b - (carry ? 1 : 0);
    if (temp > 0xFF)
        cpu->f |= FLAG_C;
    else
        cpu->f &= ~FLAG_C;
}

void clear_flags(Registers *cpu) {
    cpu->f = 0;
}

// -------------------------------------------------------------------------------------