#include "cpu.h"
#define BOOT_ROM "../bootrom/bootix_dmg.bin"

// Change Z based on result <- NOTE this is the exact opposite of all other flag functions
void set_Z(uint8_t result, CPU *cpu) {
    if (result == 0) {
        cpu->regs.f |= FLAG_Z;
    } else {
        cpu->regs.f &= ~FLAG_Z;
    }    
}

// Change N
void set_N(bool condition, CPU *cpu) {
    if (condition)
        cpu->regs.f |= FLAG_N;
    else
        cpu->regs.f &= ~FLAG_N;
}

// Set H based on condition
void set_H(bool condition, CPU *cpu) {
    if (condition)
        cpu->regs.f |= FLAG_H;
    else
        cpu->regs.f &= ~FLAG_H;
}

// Set H for ADD
void set_H_add(uint8_t a, uint8_t b, CPU *cpu) {
    if (((a & 0x0F) + (b & 0x0F)) > 0x0F)
        cpu->regs.f |= FLAG_H;
    else
        cpu->regs.f &= ~FLAG_H;
}

// Set H for SUB    
void set_H_sub(uint8_t a, uint8_t b, CPU *cpu) {
    if ((a & 0x0F) < (b & 0x0F))
        cpu->regs.f |= FLAG_H;
    else
        cpu->regs.f &= ~FLAG_H;
}

void set_H_add16(uint16_t a, uint16_t b, CPU *cpu) {
    if (((a & 0x0FFF) + (b & 0x0FFF)) > 0x0FFF)
        cpu->regs.f |= FLAG_H;
    else
        cpu->regs.f &= ~FLAG_H;
}

void set_H_inc(uint8_t before, CPU *cpu) {
    if (((before & 0x0F) + 1) > 0x0F)
        cpu->regs.f |= FLAG_H;
    else
        cpu->regs.f &= ~FLAG_H;
}

void set_H_dec(uint8_t before, CPU *cpu) {
    if ((before & 0x0F) == 0)
        cpu->regs.f |= FLAG_H;
    else
        cpu->regs.f &= ~FLAG_H;
}


// Set C based on condition
void set_C(bool condition, CPU *cpu) {
    if (condition)
        cpu->regs.f |= FLAG_C;
    else
        cpu->regs.f &= ~FLAG_C;
}

// Set C for ADD
void set_C_add(uint8_t a, uint8_t b, CPU *cpu) {
    if ((uint16_t)a + (uint16_t)b > 0xFF)
        cpu->regs.f |= FLAG_C;
    else
        cpu->regs.f &= ~FLAG_C;
}

// Set C for SUB
void set_C_sub(uint8_t a, uint8_t b, CPU *cpu) {
    if (a < b)
        cpu->regs.f |= FLAG_C;
    else
        cpu->regs.f &= ~FLAG_C;
}

void set_C_add16(uint16_t a, uint16_t b, CPU *cpu) {
    if ((uint32_t)a + (uint32_t)b > 0xFFFF)
        cpu->regs.f |= FLAG_C;
    else
        cpu->regs.f &= ~FLAG_C;
}

void set_C_sbc(uint8_t a, uint8_t b, bool carry, CPU *cpu) {
    uint16_t temp = a - b - (carry ? 1 : 0);
    if (temp > 0xFF)
        cpu->regs.f |= FLAG_C;
    else
        cpu->regs.f &= ~FLAG_C;
}

void clear_flags(CPU *cpu) {
    cpu->regs.f = 0;
}

// -------------------------------------------------------------------------------------

bool load_bootrom(CPU *cpu) {
    FILE *bootrom_file = fopen(BOOT_ROM, "rb");
    if (bootrom_file == NULL) {
        printf("Error: Could not load boot ROM.\n");
        return false;
    }

    fread(cpu->memory, 1, 0x0100, bootrom_file);  //[0x0000 to 0x00FF]

    fclose(bootrom_file);
    return true;
}



void start_cpu(CPU *cpu) {
    cpu->regs.af = 0x01B0;  // A and F initial values 
    cpu->regs.bc = 0x0013;  // B and C initial values
    cpu->regs.de = 0x00D8;  // D and E initial values
    cpu->regs.hl = 0x014D;  // H and L initial values

    // Stack pointer and program counter
    cpu->sp = 0xFFFE;
    cpu->pc = 0x0000;  

    // Initialize memory
    for (int i = 0; i < MEMORY_SIZE; ++i) {
        cpu->memory[i] = 0x00;
    }

    // Load the boot ROM 
    if (!load_bootrom(cpu)) {
        printf("Bootrom failed !!!! Aborting.");
        return;
    }

    // PPU initialization 
    ppu_init(&cpu->ppu);  

    // Interrupts and states
    cpu->ime = false;  
    cpu->ie = 0x00;  
    cpu->iflag = 0x00;  

    cpu->halted = false;
    cpu->stopped = false;

    cpu->div = 0x00;  
    cpu->tima = 0x00; 
    cpu->tma = 0x00;  
    cpu->tac = 0x00;    

    cpu->cycles = 0; 
}
