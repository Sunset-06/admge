#include "cpu.h"
#define BOOT_ROM "../bootrom/bootix_dmg.bin"

// helper for start_cpu()
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

// initializes emu state
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

    // initializes ppu state
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

/*Update all timers. Yet to decode DMG Timer behaviours, so this is just a generated function fornow*/
void update_timers(CPU *cpu, uint16_t tcycles) {
    // Update DIV - the main timer
    cpu->div += tcycles;

    if (cpu->tac & 0x04) {
        // Which bit of div controls TIMA increments
        int bit;
        switch (cpu->tac & 0x03) {
            case 0: bit = 9;  break; // 4096 Hz   (DIV bit 9)
            case 1: bit = 3;  break; // 262144 Hz (DIV bit 3)
            case 2: bit = 5;  break; // 65536 Hz  (DIV bit 5)
            case 3: bit = 7;  break; // 16384 Hz  (DIV bit 7)
        }

        // Detect falling edge of selected DIV bit
        static int prev_bit = 0;
        for (int i = 0; i < tcycles; i++) {
            int curr_bit = (cpu->div >> bit) & 1;
            if (prev_bit == 1 && curr_bit == 0) {
                cpu->tima++;
                if (cpu->tima == 0) {
                    cpu->tima = cpu->tma;
                    cpu->iflag |= (1 << 2); 
                }
            }
            prev_bit = curr_bit;
            cpu->div++;
        }
    }
}


/* Basically the main function that drives the emu. In every step, fetch opcode
    1. Fetch opcode from memory
    2. Execute the instruction 
    3. Update timers
    4. Do a PPU step
*/
void cpu_step(CPU *cpu) {
    // Fetch, decode, execute - then increment pc, cause the instructions increment it for bytes.
    uint8_t opcode = read8(cpu, cpu->pc);
    run_inst(opcode, cpu);
    cpu->pc++;

    // 2. Convert to t-cycles - then update timers and ppu
    uint16_t tcycles = cpu->cycles * 4;
    update_timers(cpu, tcycles);
    ppu_step(&cpu->ppu, cpu, tcycles);

    //handle_interrupts(cpu);
}

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