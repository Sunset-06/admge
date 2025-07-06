#include "cpu.h";

// Change Zero Flag
void set_Z(uint8_t result, Registers *cpu) {
    if (result == 0) {
        cpu->f |= FLAG_Z;
    } else {
        cpu->f &= ~FLAG_Z;
    }
}

// Clear Negative Flag
void clear_N(Registers *cpu) {
    cpu->f &= ~FLAG_N;
}

// Set Negative flag
void set_N(Registers *cpu) {
    cpu->f |= FLAG_N;
}

// clear flag state
void clear_flags(Registers *cpu) {
    cpu->f = 0;  
}

void set_C(uint8_t val, Registers *cpu){
    cpu->f = val;
}

// -------------------------------------------------------------------------------------