#include "cpu.h"
#include "emu.h"

// helper: get pointer to register by index
static inline uint8_t* get_register(CPU *cpu, int reg_index) {
    switch (reg_index) {
        case 0: return &cpu->regs.b;
        case 1: return &cpu->regs.c;
        case 2: return &cpu->regs.d;
        case 3: return &cpu->regs.e;
        case 4: return &cpu->regs.h;
        case 5: return &cpu->regs.l;
        case 7: return &cpu->regs.a;
        default: return NULL; // index 6 means (HL), handled separately
    }
}

void execute_RES(CPU *cpu, uint8_t opcode) {
    int bit = (opcode - 0x80) / 8;   // which bit to reset (0–7)
    int reg_index = (opcode - 0x80) % 8; // which register/(HL)

    if (reg_index == 6) {
        // (HL) case: memory access
        uint8_t val = read8(cpu, cpu->regs.hl);
        val &= ~(1 << bit);
        write8(cpu, cpu->regs.hl, val);
        cpu->cycles += 4;
    } else {
        uint8_t *reg = get_register(cpu, reg_index);
        *reg &= ~(1 << bit);
        cpu->cycles += 2;
    }
}

void execute_SET(CPU *cpu, uint8_t opcode) {
    int bit = (opcode - 0xC0) / 8;      // which bit to set (0–7)
    int reg_index = (opcode - 0xC0) % 8; // which register/(HL)

    if (reg_index == 6) {
        // (HL) case: memory access
        uint8_t val = read8(cpu, cpu->regs.hl);
        val |= (1 << bit);
        write8(cpu, cpu->regs.hl, val);
        cpu->cycles += 4;
    } else {
        uint8_t *reg = get_register(cpu, reg_index);
        *reg |= (1 << bit);
        cpu->cycles += 2;
    }
}


void run_pref_inst(CPU *cpu, uint8_t opcode){
    Registers *reg = &cpu->regs;
    uint8_t u8;
    uint8_t temp8;
    //uint16_t u16;

    switch (opcode)
    {
        case 0x00:  //RLC B
            u8 = (reg->b >> 7) & 1;
            reg->b = (reg->b << 1) | u8;
            set_Z(reg->b, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x01:  //RLC C
            u8 = (reg->c >> 7) & 1;
            reg->c = (reg->c << 1) | u8;
            set_Z(reg->c, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x02:  //RLC D
            u8 = (reg->d >> 7) & 1;
            reg->d = (reg->d << 1) | u8;
            set_Z(reg->d, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x03:  //RLC E
            u8 = (reg->e >> 7) & 1;
            reg->e = (reg->e << 1) | u8;
            set_Z(reg->e, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x04:  //RLC H
            u8 = (reg->h >> 7) & 1;
            reg->h = (reg->h << 1) | u8;
            set_Z(reg->h, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x05:  //RLC L
            u8 = (reg->l >> 7) & 1;
            reg->l = (reg->l << 1) | u8;
            set_Z(reg->l, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x06:  //RLC [HL]
            u8 = read8(cpu, reg->hl);
            u8 = (u8 << 1) | (u8 >> 7);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C((u8 & 0x01) != 0, cpu);
            write8(cpu, reg->hl, u8);
            cpu->cycles += 2;
            break;

        case 0x07:  //RLC A
            u8 = (reg->a >> 7) & 1; // recording the msb
            reg->a = (reg->a << 1) | u8;
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x08:  //RRC B
            u8 = reg->b & 0x01; // recording lsb
            reg->b = (reg->b >> 1) | (u8 << 7);
            set_Z(reg->b, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x09:  //RRC C
            u8 = reg->c & 0x01; 
            reg->c = (reg->c >> 1) | (u8 << 7);
            set_Z(reg->c, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x0A:  //RRC D
            u8 = reg->d & 0x01; 
            reg->d = (reg->d >> 1) | (u8 << 7);
            set_Z(reg->d, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;
        
        case 0x0B:  //RRC E
            u8 = reg->e & 0x01; 
            reg->e = (reg->e >> 1) | (u8 << 7);
            set_Z(reg->e, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x0C:  //RRC H
            u8 = reg->h & 0x01; 
            reg->h = (reg->h >> 1) | (u8 << 7);
            set_Z(reg->h, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x0D:  //RRC L
            u8 = reg->l & 0x01; 
            reg->l = (reg->l >> 1) | (u8 << 7);
            set_Z(reg->l, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x0E:  //RRC [HL]
            uint8_t lsb = read8(cpu, reg->hl) & 0x01;
            u8 = read8(cpu, reg->hl);
            u8 = (u8 >> 1) | (lsb << 7);
            write8(cpu, reg->hl, u8);     
            set_Z(u8, cpu);
            set_N(0, cpu);                         
            set_H(0, cpu);                         
            set_C(lsb, cpu);
            cpu->cycles += 2;  
            break;

        case 0x0F:  //RRC A
            u8 = reg->a & 0x01; 
            reg->a = (reg->a >> 1) | (u8 << 7);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x10:  //RL B
            u8 = (reg->b >> 7) & 1;
            reg->b = (reg->b << 1) | ((reg->f & FLAG_C)? 1 : 0);
            set_Z(reg->b, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x11:  //RL C
            u8 = (reg->c >> 7) & 1;
            reg->c = (reg->c << 1) | ((reg->f & FLAG_C)? 1 : 0);
            set_Z(reg->c, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x12:  //RL D
            u8 = (reg->d >> 7) & 1;
            reg->d = (reg->d << 1) | ((reg->f & FLAG_C)? 1 : 0);
            set_Z(reg->d, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x13:  //RL E
            u8 = (reg->e >> 7) & 1;
            reg->e = (reg->e << 1) | ((reg->f & FLAG_C)? 1 : 0);
            set_Z(reg->e, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x14:  //RL H
            u8 = (reg->h >> 7) & 1;
            reg->h = (reg->h << 1) | ((reg->f & FLAG_C)? 1 : 0);
            set_Z(reg->h, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;
        
        case 0x15:  //RL L
            u8 = (reg->l >> 7) & 1;
            reg->l = (reg->l << 1) | ((reg->f & FLAG_C)? 1 : 0);
            set_Z(reg->l, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x16:  //RL [HL]
            temp8 = read8(cpu, reg->hl);
            u8 = (temp8 >> 7) & 1;
            temp8 = (temp8 << 1) | ((reg->f & FLAG_C)? 1 : 0);
            write8(cpu, reg->hl, temp8);
            set_Z(temp8, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;
        
        case 0x17:  //RL A
            u8 = (reg->a >> 7) & 1;
            reg->a = (reg->a << 1) | ((reg->f & FLAG_C)? 1 : 0);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;
        
        case 0x18:  //RR B
            temp8 = (reg->f & FLAG_C) ? 0x80 : 0x00; // bit in carry
            u8 = reg->b & 0x01; // lsb
            reg->b = (reg->b >> 1) | temp8; // add carry to the msb
            set_Z(reg->b, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x19:  //RR C 
            temp8 = (reg->f & FLAG_C) ? 0x80 : 0x00; // bit in carry
            u8 = reg->c & 0x01; // lsb
            reg->c = (reg->c >> 1) | temp8; // add carry to the msb
            set_Z(reg->c, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;
        
        case 0x1A:  //RR D
            temp8 = (reg->f & FLAG_C) ? 0x80 : 0x00; // bit in carry
            u8 = reg->d & 0x01; // lsb
            reg->d = (reg->d >> 1) | temp8; // add carry to the msb
            set_Z(reg->d, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x1B:  //RR E
            temp8 = (reg->f & FLAG_C) ? 0x80 : 0x00; 
            u8 = reg->e & 0x01; 
            reg->e = (reg->e >> 1) | temp8; 
            set_Z(reg->e, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x1C:  //RR H
            temp8 = (reg->f & FLAG_C) ? 0x80 : 0x00; 
            u8 = reg->h & 0x01; 
            reg->h = (reg->h >> 1) | temp8; 
            set_Z(reg->h, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x1D:  //RR L
            temp8 = (reg->f & FLAG_C) ? 0x80 : 0x00;
            u8 = reg->l & 0x01; 
            reg->l = (reg->l >> 1) | temp8; 
            set_Z(reg->l, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;
        
        case 0x1E:  //RR [HL] 
            u8 = read8(cpu, reg->hl);
            temp8 = u8 & 0x01;
            u8 = (u8 >> 1) | (reg->f & FLAG_C) ? 0x80 : 0x00;
            write8(cpu, reg->hl, u8);  
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(temp8, cpu);
            cpu->cycles += 2;
            break;

        case 0x1F:  //RR A
            temp8 = (reg->f & FLAG_C) ? 0x80 : 0x00; 
            u8 = reg->a & 0x01; 
            reg->a = (reg->a >> 1) | temp8; 
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x20:  // SLA B
            u8 = (reg->b >> 7) & 1;
            reg->b = (reg->b << 1);
            set_Z(reg->b, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x21:  // SLA C
            u8 = (reg->c >> 7) & 1;
            reg->c = (reg->c << 1);
            set_Z(reg->c, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x22:  // SLA D
            u8 = (reg->d >> 7) & 1;
            reg->d = (reg->d << 1);
            set_Z(reg->d, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x23:  // SLA E
            u8 = (reg->e >> 7) & 1;
            reg->e = (reg->e << 1);
            set_Z(reg->e, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x24:  // SLA H
            u8 = (reg->h >> 7) & 1;
            reg->h = (reg->h << 1);
            set_Z(reg->h, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x25:  // SLA L
            u8 = (reg->l >> 7) & 1;
            reg->l = (reg->l << 1);
            set_Z(reg->l, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x26:  // SLA (HL)
            temp8 = read8(cpu, reg->hl);
            u8 = (temp8 >> 7) & 1;
            temp8 = (temp8 << 1);
            write8(cpu, reg->hl, temp8);
            set_Z(temp8, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x27:  // SLA A
            u8 = (reg->a >> 7) & 1;
            reg->a = (reg->a << 1);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x28:  // SRA B
            u8 = reg->b & 1;
            reg->b = (reg->b >> 1) | (reg->b & 0x80);
            set_Z(reg->b, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x29:  // SRA C
            u8 = reg->c & 1;
            reg->c = (reg->c >> 1) | (reg->c & 0x80);
            set_Z(reg->c, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x2A:  // SRA D
            u8 = reg->d & 1;
            reg->d = (reg->d >> 1) | (reg->d & 0x80);
            set_Z(reg->d, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x2B:  // SRA E
            u8 = reg->e & 1;
            reg->e = (reg->e >> 1) | (reg->e & 0x80);
            set_Z(reg->e, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x2C:  // SRA H
            u8 = reg->h & 1;
            reg->h = (reg->h >> 1) | (reg->h & 0x80);
            set_Z(reg->h, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x2D:  // SRA L
            u8 = reg->l & 1;
            reg->l = (reg->l >> 1) | (reg->l & 0x80);
            set_Z(reg->l, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x2E:  // SRA (HL)
            temp8 = read8(cpu, reg->hl);
            u8 = temp8 & 1;
            temp8 = (temp8 >> 1) | (temp8 & 0x80);
            write8(cpu, reg->hl, temp8);
            set_Z(temp8, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x2F:  // SRA A
            u8 = reg->a & 1;
            reg->a = (reg->a >> 1) | (reg->a & 0x80);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;
        
        case 0x30:  // SWAP B
            reg->b = ((reg->b << 4) | (reg->b >> 4)) & 0xFF;
            set_Z(reg->b, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0x31:  // SWAP C
            reg->c = ((reg->c << 4) | (reg->c >> 4)) & 0xFF;
            set_Z(reg->c, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0x32:  // SWAP D
            reg->d = ((reg->d << 4) | (reg->d >> 4)) & 0xFF;
            set_Z(reg->d, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0x33:  // SWAP E
            reg->e = ((reg->e << 4) | (reg->e >> 4)) & 0xFF;
            set_Z(reg->e, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0x34:  // SWAP H
            reg->h = ((reg->h << 4) | (reg->h >> 4)) & 0xFF;
            set_Z(reg->h, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0x35:  // SWAP L
            reg->l = ((reg->l << 4) | (reg->l >> 4)) & 0xFF;
            set_Z(reg->l, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0x36:  // SWAP (HL)
            temp8 = read8(cpu, reg->hl);
            temp8 = ((temp8 << 4) | (temp8 >> 4)) & 0xFF;
            write8(cpu, reg->hl, temp8);
            set_Z(temp8, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->cycles += 2;
            break;

        case 0x37:  // SWAP A
            reg->a = ((reg->a << 4) | (reg->a >> 4)) & 0xFF;
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0x38:  // SRL B
            u8 = reg->b & 1;
            reg->b = (reg->b >> 1);
            set_Z(reg->b, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x39:  // SRL C
            u8 = reg->c & 1;
            reg->c = (reg->c >> 1);
            set_Z(reg->c, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x3A:  // SRL D
            u8 = reg->d & 1;
            reg->d = (reg->d >> 1);
            set_Z(reg->d, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;


        case 0x3B:  // SRL E
            u8 = reg->e & 1;
            reg->e = (reg->e >> 1);
            set_Z(reg->e, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x3C:  // SRL H
            u8 = reg->h & 1;
            reg->h = (reg->h >> 1);
            set_Z(reg->h, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x3D:  // SRL L
            u8 = reg->l & 1;
            reg->l = (reg->l >> 1);
            set_Z(reg->l, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x3E:  // SRL HL
            temp8 = read8(cpu, reg->hl);
            u8 = temp8 & 1;
            temp8 = (temp8 >> 1);
            write8(cpu, reg->hl, temp8);
            set_Z(temp8, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;

        case 0x3F:  // SRL A
            u8 = reg->a & 1;
            reg->a = (reg->a >> 1);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            break;
        
        case 0x40:  // BIT 0,B
            u8 = ((reg->b) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
        
        case 0x41:  // BIT 0,C
            u8 = ((reg->c) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
        
        case 0x42:  // BIT 0,D
            u8 = ((reg->d) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
        
        case 0x43:  // BIT 0,E
            u8 = ((reg->e) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
        
        case 0x44:  // BIT 0,H
            u8 = ((reg->h) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
        
        case 0x45:  // BIT 0,L
            u8 = ((reg->l) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
        
        case 0x46:  // BIT 0,HL
            u8 = ((read8(cpu, reg->hl)) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
        
        case 0x47:  // BIT 0,A
            u8 = ((reg->a) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
        
        case 0x48:  // BIT 1,B
            u8 = ((reg->a >> 1) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
        
        case 0x49:  // BIT 1,C
            u8 = ((reg->c >> 1) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
        
        case 0x4A:  // BIT 1,D
            u8 = ((reg->d >> 1) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
        
        case 0x4B:  // BIT 1,E
            u8 = ((reg->e >> 1) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
        
        case 0x4C:  // BIT 1,H
            u8 = ((reg->h >> 1) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
        
        case 0x4D:  // BIT 1,L
            u8 = ((reg->l >> 1) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
        
        case 0x4E:  // BIT 1,HL
            u8 = ((read8(cpu, reg->hl) >> 1) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
        
        case 0x4F:  // BIT 1,A
            u8 = ((reg->a >> 1) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
        
        case 0x50:  // BIT 2,B
            u8 = ((reg->a >> 2) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;  
        
        case 0x51:  // BIT 2,C
            u8 = ((reg->c >> 2) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x52:  // BIT 2,D
            u8 = ((reg->d >> 2) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x53:  // BIT 2,E
            u8 = ((reg->e >> 2) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x54:  // BIT 2,H
            u8 = ((reg->h >> 2) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x55:  // BIT 2,L
            u8 = ((reg->l >> 2) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x56:  // BIT 2,HL
            u8 = ((read8(cpu, reg->hl) >> 2) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x57:  // BIT 2,A
            u8 = ((reg->a >> 2) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x58:  // BIT 3,B
            u8 = ((reg->b >> 3) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x59:  // BIT 3,C
            u8 = ((reg->c >> 3) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x5A:  // BIT 3,D
            u8 = ((reg->d >> 3) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x5B:  // BIT 3,E
            u8 = ((reg->e >> 3) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;  
                    
        case 0x5C:  // BIT 3,H
            u8 = ((reg->h >> 3) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x5D:  // BIT 3,L
            u8 = ((reg->l >> 3) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x5E:  // BIT 3,HL
            u8 = ((read8(cpu, reg->hl) >> 3) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x5F:  // BIT 3,A
            u8 = ((reg->a >> 3) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x60:  // BIT 4,B
            u8 = ((reg->b >> 4) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x61:  // BIT 4,C
            u8 = ((reg->c >> 4) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x62:  // BIT 4,D
            u8 = ((reg->d >> 4) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x63:  // BIT 4,E
            u8 = ((reg->e >> 4) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x64:  // BIT 4,H
            u8 = ((reg->h >> 4) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x65:  // BIT 4,L
            u8 = ((reg->l >> 4) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
                    
        case 0x66:  // BIT 4,HL
            u8 = ((read8(cpu, reg->hl) >> 4) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

                    
        case 0x67:  // BIT 4,A
            u8 = ((reg->a >> 4) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x68:  // BIT 5,B
            u8 = ((reg->b >> 5) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x69:  // BIT 5,C
            u8 = ((reg->c >> 5) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x6A:  // BIT 5,D
            u8 = ((reg->d >> 5) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x6B:  // BIT 5,E
            u8 = ((reg->e >> 5) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x6C:  // BIT 5,H
            u8 = ((reg->h >> 5) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x6D:  // BIT 5,L
            u8 = ((reg->l >> 5) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x6E:  // BIT 5,HL
            u8 = ((read8(cpu, reg->hl) >> 5) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x6F:  // BIT 5,A
            u8 = ((reg->a >> 5) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x70:  // BIT 6,B
            u8 = ((reg->b >> 6) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x71:  // BIT 6,C
            u8 = ((reg->c >> 6) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x72:  // BIT 6,D
            u8 = ((reg->d >> 6) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x73:  // BIT 6,E
            u8 = ((reg->e >> 6) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x74:  // BIT 6,H
            u8 = ((reg->h >> 6) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x75:  // BIT 6,L
            u8 = ((reg->l >> 6) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x76:  // BIT 6,HL
            u8 = ((read8(cpu, reg->hl) >> 6) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x77:  // BIT 6,A
            u8 = ((reg->a >> 6) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x78:  //BIT 7, B
            u8 = ((reg->b >> 7) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x79:  //BIT 7, C
            u8 = ((reg->c >> 7) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x7A:  //BIT 7, D
            u8 = ((reg->d >> 7) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x7B:  //BIT 7, E
            u8 = ((reg->e >> 7) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x7C:  //BIT 7, H
            u8 = ((reg->h >> 7) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x7D:  //BIT 7, L
            u8 = ((reg->l >> 7) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x7E:  //BIT 7, HL
            u8 = ((read8(cpu, reg->hl) >> 7) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;

        case 0x7F:  //BIT 7, A
            u8 = ((reg->a >> 7) & 1);
            set_Z(u8, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            break;
        
        case 0x80 ... 0xBF:  // RES instructions
            execute_RES(cpu, opcode);
            break;

        case 0xC0 ... 0xFF:  // SET instructions
            execute_SET(cpu, opcode);
            break;

    }
}