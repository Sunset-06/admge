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

// Set or clear Half Carry flag 
void set_H(uint8_t value1, uint8_t value2, uint8_t result, Registers *cpu) {
    if (((value1 & 0x0F) + (value2 & 0x0F)) > 0x0F || 
        ((result & 0x0F) < (value1 & 0x0F))) {
        cpu->f |= FLAG_H;
    } else {
        cpu->f &= ~FLAG_H;
    }
}

// Set or clear Carry flag based on result
void set_C(uint16_t result, Registers *cpu) {
    if (result > 0xFF) {
        cpu->f |= FLAG_C;
    } else {
        cpu->f &= ~FLAG_C;
    }
}

// Set Carry flag for rotate operations
void set_C_otate(uint8_t value, Registers *cpu) {
    if (value & 0x80) {  // Check if MSB is set
        cpu->f |= FLAG_C;
    } else {
        cpu->f &= ~FLAG_C;
    }
}

// Change carry flag for subtraction
void set_C_sub(uint8_t value1, uint8_t value2, Registers *cpu) {
    if (value1 < value2) {
        cpu->f |= FLAG_C; 
    } else {
        cpu->f &= ~FLAG_C;
    }
}

// clear flag state
void clear_flags(Registers *cpu) {
    cpu->f = 0;  
}
