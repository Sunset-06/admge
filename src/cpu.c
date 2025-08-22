#include "cpu.h"
#include "emu.h"

// initializes emu state
void start_cpu(CPU *cpu) {
    ////printf("Starting CPU init\n");
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

    // initializes ppu state
    ppu_init(&cpu->ppu);  
    cpu->bootrom_flag = true;
    // Interrupts and states
    cpu->ime = false;  
    cpu->ie = 0x00;  
    cpu->iflag = 0x00;  

    cpu->halted = false;
    cpu->stopped = false;

    cpu->div = 0x00;  
    cpu->tima = 0x00; 
    cpu->tma = 0x00;  
    cpu->tac = 0xF8;    

    cpu->cycles = 0;
}

void handle_interrupts(CPU *cpu) {
    // Acknowledge a pending interrupt
    printf("iflag / ie / ime : %04x / %04x / %04x\n", cpu->iflag, cpu->ie, cpu->ime);
    uint8_t pending = cpu->iflag & cpu->ie;
    
    if (cpu->halted && pending) {
        cpu->halted = false;
    }

    if (!cpu->ime) {
        printf("No ime this step. continue\n\n");
        return;
    }
    printf("Interrupt signal. Pending: %04x\n",pending);

    //VBlank
    if (pending & 0x01) {
        printf("Handling the vblank");
        // Clear the V-Blank interrupt flag
        cpu->iflag &= ~0x01;
        printf("Iflag cleared: %04x",  cpu->iflag);
        // Disable global interrupts
        cpu->ime = false;
        stack_push(cpu, cpu->pc);
        // Jump to the V-Blank interrupt vector
        cpu->pc = 0x40;
        cpu->cycles += 5;
    }
    // LCD STAT (bit 1)
    else if (pending & 0x02) {
        cpu->iflag &= ~0x02;
        cpu->ime = false;
        stack_push(cpu, cpu->pc);
        cpu->pc = 0x48;
        cpu->cycles += 5;
    }
    // Timer (bit 2)
    else if (pending & 0x04) {
        printf("Timer interrupt handler");
        cpu->iflag &= ~0x04;
        cpu->ime = false;
        stack_push(cpu, cpu->pc);
        cpu->pc = 0x50;
        cpu->cycles += 5;
    }
    // Serial (bit 3)
    else if (pending & 0x08) {
        cpu->iflag &= ~0x08;
        cpu->ime = false;
        stack_push(cpu, cpu->pc);
        cpu->pc = 0x58;
        cpu->cycles += 5;
    }
    // Joypad (bit 4)
    else if (pending & 0x10) {
        cpu->iflag &= ~0x10;
        cpu->ime = false;
        stack_push(cpu, cpu->pc);
        cpu->pc = 0x60;
        cpu->cycles += 5;
    }
}

/*
* Add to your CPU struct:
* int timer_counter;
*/

void update_timers(CPU *cpu, uint16_t tcycles) {
    // Update DIV counter (internal 16-bit counter)
    cpu->div += tcycles;

    // Check if the timer is enabled in the TAC register (bit 2)
    if (!(cpu->tac & 0x04)) {
        return;
    }

    // Add cycles to the TIMA accumulator
    timer_counter += tcycles;

    // Get the cycle threshold for the current frequency
    uint16_t rate_cycles = 0;
    switch (cpu->tac & 0x03) {
        case 0: rate_cycles = 1024; break; 
        case 1: rate_cycles = 16;   break; 
        case 2: rate_cycles = 64;   break; 
        case 3: rate_cycles = 256;  break; 
    }

    while (timer_counter >= rate_cycles) {
        timer_counter -= rate_cycles;

        // Check if TIMA will overflow
        if (cpu->tima == 0xFF) {
            cpu->tima = cpu->tma; ///reset TIMA to TMA
            cpu->iflag |= 0x04; 
        } else {
            cpu->tima++; 
        }
    }
}


/* Basically the main function that drives the emu. In every step, fetch opcode
    1. Fetch opcode from memory
    2. Execute the instruction 
    3. Update timers
    4. Do a PPU step
*/
void cpu_step(CPU *cpu){
    if (cpu->ime_enable) {
        printf("ime_enable hit true. Enabling ime now\n");
        cpu->ime = true;
        cpu->ime_enable = false;
    }
    
    if (cpu->halted) {
        //printf("The CPU was halted!\n\n");
        handle_interrupts(cpu);
        return;
    }

    //printf("Starting a step.\n");
    uint8_t opcode = read8(cpu, cpu->pc);
    printf("Current op: %02x \n", opcode);
    run_inst(opcode, cpu);
    //printf("pc post inst %02x \n\n", cpu->pc);
    ppu_step(&cpu->ppu, cpu);
    update_timers(cpu, cpu->cycles*4);
    cpu->cycles = 0;
    handle_interrupts(cpu);
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