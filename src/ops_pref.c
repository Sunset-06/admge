#include "cpu.h"
#include "emu.h"

void run_pref_inst(CPU *cpu){
    uint16_t opcode = (cpu, ++cpu->pc);
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
            cpu->cycles += 2;
            break;

        case 0x01:  //RLC C
            u8 = (reg->c >> 7) & 1;
            reg->c = (reg->c << 1) | u8;
            set_Z(reg->c, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x02:  //RLC D
            u8 = (reg->d >> 7) & 1;
            reg->d = (reg->d << 1) | u8;
            set_Z(reg->d, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x03:  //RLC E
            u8 = (reg->e >> 7) & 1;
            reg->e = (reg->e << 1) | u8;
            set_Z(reg->e, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x04:  //RLC H
            u8 = (reg->h >> 7) & 1;
            reg->h = (reg->h << 1) | u8;
            set_Z(reg->h, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x05:  //RLC L
            u8 = (reg->l >> 7) & 1;
            reg->l = (reg->l << 1) | u8;
            set_Z(reg->l, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
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
            cpu->cycles += 2;
            break;

        case 0x08:  //RRC B
            u8 = reg->b & 0x01; // recording lsb
            reg->b = (reg->b >> 1) | (u8 << 7);
            set_Z(reg->b, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x09:  //RRC C
            u8 = reg->c & 0x01; 
            reg->c = (reg->c >> 1) | (u8 << 7);
            set_Z(reg->c, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x0A:  //RRC D
            u8 = reg->d & 0x01; 
            reg->d = (reg->d >> 1) | (u8 << 7);
            set_Z(reg->d, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;
        
        case 0x0B:  //RRC E
            u8 = reg->e & 0x01; 
            reg->e = (reg->e >> 1) | (u8 << 7);
            set_Z(reg->e, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x0C:  //RRC H
            u8 = reg->h & 0x01; 
            reg->h = (reg->h >> 1) | (u8 << 7);
            set_Z(reg->h, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x0D:  //RRC L
            u8 = reg->l & 0x01; 
            reg->l = (reg->l >> 1) | (u8 << 7);
            set_Z(reg->l, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x0E:  //RRC [HL]
            uint8_t lsb = read8(cpu, reg->hl) & 0x01;
            u8 = read8(cpu, reg->hl);
            u8 = (u8 >> 1) | (lsb << 7);
            write8(cpu, reg->hl, u8);     
            set_Z(u8, cpu);
            set_N(0, cpu);                         
            set_H(0, cpu);                         
            set_C(u8, cpu);
            cpu->cycles += 2;  
            break;

        case 0x0F:  //RRC A
            u8 = reg->a & 0x01; 
            reg->a = (reg->a >> 1) | (u8 << 7);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x10:  //RL B
            u8 = (reg->b >> 7) & 1;
            reg->b = (reg->b << 1) | ((reg->f & FLAG_C)? 1 : 0);
            set_Z(reg->b, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x11:  //RL C
            u8 = (reg->c >> 7) & 1;
            reg->c = (reg->c << 1) | ((reg->f & FLAG_C)? 1 : 0);
            set_Z(reg->b, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x12:  //RL D
            u8 = (reg->d >> 7) & 1;
            reg->d = (reg->d << 1) | ((reg->f & FLAG_C)? 1 : 0);
            set_Z(reg->d, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x13:  //RL E
            u8 = (reg->e >> 7) & 1;
            reg->e = (reg->e << 1) | ((reg->f & FLAG_C)? 1 : 0);
            set_Z(reg->e, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x14:  //RL H
            u8 = (reg->h >> 7) & 1;
            reg->h = (reg->h << 1) | ((reg->f & FLAG_C)? 1 : 0);
            set_Z(reg->h, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;
        
        case 0x15:  //RL L
            u8 = (reg->l >> 7) & 1;
            reg->l = (reg->l << 1) | ((reg->f & FLAG_C)? 1 : 0);
            set_Z(reg->l, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x16:  //RL [HL]
            temp8 = read8(cpu, reg->hl);
            u8 = (temp8 >> 7) & 1;
            temp8 = (temp8 << 1) | ((reg->f & FLAG_C)? 1 : 0);
            set_Z(temp8, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 4;
            break;
        
        case 0x17:  //RL A
            u8 = reg->a & 0x01;
            reg->a = (reg->a << 1) | ((reg->f & FLAG_C)? 1 : 0);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;
        
        case 0x18:  //RR B
            temp8 = (reg->f & FLAG_C) ? 0x80 : 0x00; // bit in carry
            u8 = reg->b & 0x01; // lsb
            reg->b = (reg->b >> 1) | temp8; // add carry to the msb
            set_Z(0, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x19:  //RR C 
            temp8 = (reg->f & FLAG_C) ? 0x80 : 0x00; // bit in carry
            u8 = reg->c & 0x01; // lsb
            reg->c = (reg->c >> 1) | temp8; // add carry to the msb
            set_Z(0, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;
        
        case 0x1A:  //RR D
            temp8 = (reg->f & FLAG_C) ? 0x80 : 0x00; // bit in carry
            u8 = reg->d & 0x01; // lsb
            reg->d = (reg->d >> 1) | temp8; // add carry to the msb
            set_Z(0, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x1B:  //RR E
            temp8 = (reg->f & FLAG_C) ? 0x80 : 0x00; 
            u8 = reg->e & 0x01; 
            reg->e = (reg->e >> 1) | temp8; 
            set_Z(0, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x1C:  //RR H
            temp8 = (reg->f & FLAG_C) ? 0x80 : 0x00; 
            u8 = reg->h & 0x01; 
            reg->h = (reg->h >> 1) | temp8; 
            set_Z(0, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;

        case 0x1D:  //RR L
            temp8 = (reg->f & FLAG_C) ? 0x80 : 0x00;
            u8 = reg->l & 0x01; 
            reg->l = (reg->l >> 1) | temp8; 
            set_Z(0, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;
        
        case 0x1E:  //RR [HL] 
            u8 = read8(cpu, reg->hl);
            temp8 = u8 & 0x01;
            u8 = (u8 >> 1) | (reg->f & FLAG_C) ? 0x80 : 0x00;
            write8(cpu, reg->hl, u8);  
            set_Z(0, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(temp8, cpu);
            cpu->cycles += 2;
            break;

        case 0x1F:  //RR A
            temp8 = (reg->f & FLAG_C) ? 0x80 : 0x00; 
            u8 = reg->a & 0x01; 
            reg->a = (reg->a >> 1) | temp8; 
            set_Z(0, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->cycles += 2;
            break;
    }
}