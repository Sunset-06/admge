#include "cpu.h"
#include "emu.h"

void run_pref_inst(CPU *cpu){
    uint16_t opcode = (cpu, ++cpu->pc);
    Registers *reg = &cpu->regs;
    uint8_t u8;
    uint16_t u16;

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
            u8 = read_memory(cpu, reg->hl);
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
    }
}