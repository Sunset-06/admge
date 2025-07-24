#include "cpu.h"
#include "emu.h"
// TODO- Check set_Z implementation. You need to pass 1 to unset, which is reeally unintuitive

void run_inst(uint16_t opcode, CPU *cpu){
    // temporary variables
    uint16_t u16;
    uint8_t u8;
    uint8_t temp8;
    uint16_t temp16;
    int8_t offset;
    Registers *reg = &cpu->regs;

    switch(opcode){
        case 0x00:  //NOP
            break;

        case 0x01:  //LD BC, u16 - 3bytes
            // write u16 into BC
            u16 = cpu->memory[++cpu->pc];
            u16 = u16 | (cpu->memory[++cpu->pc] << 8);
            cpu->bc = u16;
            break;

        case 0x02:  //LD BC, A 
            //write from A to byte pointed by BC
            cpu->memory[cpu->bc] =  cpu->a;    
            break;

        case 0x03:  //INC BC
            //Increment value of BC by 1
            cpu->bc += 1;
            break;

        case 0x04:  //INC B
            //Increment value of B by 1
            set_H_add(cpu->b, 1, cpu);
            set_N(0,cpu);
            cpu->b += 1;
            set_Z(cpu->b, cpu);
            break;

        case 0x05:  //DEC B
            // Decrease value of B by 1
            set_H_sub(cpu->b, 1, cpu);
            cpu->b -= 1;
            // Change Z and set N flag
            set_Z(cpu->b, cpu);
            set_N(cpu->b, cpu);
            break;

        case 0x06:  //LD B, u8
            // Write u8 into B
            cpu->b = cpu->memory[++cpu->pc];
            break;
            
        case 0x07:  //RLCA
            // Rotate Left A
            u8 = (cpu->a >> 7) & 1; // recording the msb
            set_Z(1, cpu);   // passing 1 basically unsets it 
            set_N(0, cpu); 
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->a = cpu->a << 1;
            cpu->a = cpu->a | u8;
            break;

        case 0x08:  //LD u16, SP     
            // Copy SP & $FF at address u16 and SP >> 8 at address u16 + 1.
            u16 = cpu->memory[++cpu->pc];
            u16 |= (cpu->memory[++cpu->pc] << 8);
            cpu->memory[u16] = cpu->sp & 0xFF;
            cpu->memory[u16 + 1] = (cpu->sp >> 8) & 0xFF;
            break;

        case 0x09:  //ADD HL, BC
            // Add value of BC into HL
            set_N(0, cpu);
            set_H_add(cpu->hl, cpu->bc, cpu);
            set_C_add(cpu->hl, cpu->bc, cpu);
            cpu->hl += cpu->bc;
            break;

        case 0x0A:  //LD A, BC
            // Write Byte pointed by BC into A
            cpu->a = cpu->memory[cpu->bc];
            break;
        
        case 0x0B:  //DEC BC
            // Decrement value in BC by 1
            cpu->bc -= 1;
            break;  

        case 0x0C:  //INC C
            // Increment value in C by 1
            set_N(0, cpu);
            set_H_add(cpu->c, 1,cpu);
            cpu->c += 1;
            set_Z(cpu->c, cpu);
            break;  

        case 0x0D:  //DEC C
            // Decrement value in C by 1
            set_N(1, cpu);
            set_H_sub(cpu->c, 1, cpu);
            cpu->c -= 1;
            set_Z(cpu->c, cpu);
            break;  

        case 0x0E:  //LD C, u8
            // Write u8 into C
            cpu->c = cpu->memory[++cpu->pc];
            break;  

        case 0x0F:  //RRCA
            // Rotate Register A right
            set_Z(1, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            u8 = cpu->a & 0x01; // recording lsb
            set_C(u8, cpu);
            cpu->a = (cpu->a >> 1) | (u8 << 7);
            break;
            
        case 0x10:  //STOP
            // STOP - Check ths one before implementing
            break;

        case 0x11:  //LD DE, u16
            // Load u16 into DE
            u16 = cpu->memory[++cpu->pc];
            u16 = u16 | (cpu->memory[++cpu->pc] << 8);
            cpu->de = u16;
            break;

        case 0x12:  //LD DE, A
            // Copy the value in register A into the byte pointed to by DE
            cpu->memory[cpu->de] = cpu->a;
            break;

        case 0x13:  //INC DE
            // Increment value in DE by 1
            cpu->de += 1;
            break;

        case 0x14:  //INC D
            // Incremenrt value in D by 1
            set_N(0, cpu);  
            set_H_add(cpu->d, 1, cpu);
            cpu->d += 1;
            set_Z(cpu->d, cpu);
            break;

        case 0x15:  //DEC D
            // Decrease value in D by 1
            set_Z(cpu->d - 1, cpu);
            set_N(1, cpu);  
            set_H_sub(cpu->d, 1, cpu);
            cpu->d -= 1;
            break;
        
        case 0x16:  //LD D, u8
            // Load u8 into D
            cpu->d = cpu->memory[++cpu->pc];
            break;
        
        case 0x17:  //RLA
            // Rotate A left through carry
            set_Z(1, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            // put msb into carry
            // then put carry into lsb
            u8 = (cpu->a >> 7) & 1; 
            temp8 = (cpu->f & FLAG_C)? 1 : 0;
            set_C(u8, cpu);
            cpu->a = cpu->a << 1;
            cpu->a = cpu->a | temp8;
            break;

        case 0x18:  //JR i8
            // Jump relative by i8 steps in pc
            offset = (int8_t) cpu->memory[++cpu->pc];
            cpu->pc =+ offset; 
            break;
        
        case 0x19:  //ADD HL, DE
            // Add value of DE to HL
            set_N(0, cpu);
            set_H_add16(cpu->hl, cpu->de, cpu);
            set_C_add16(cpu->hl, cpu->de, cpu);
            cpu->hl += cpu->de;
            break;

        case 0x1A:  //LD A, DE
            // Copy Byte pointed by DE into A
            cpu->a = cpu->memory[cpu->de];
            break;
        
        case 0x1B:  //DEC DE
            // decrement de
            cpu->de -= 1;
            break;

        case 0x1C:  //INC E
            // Increment e 
            set_N(0, cpu);  
            set_H_add(cpu->e, 1, cpu);
            cpu->e += 1;
            set_Z(cpu->e, cpu);
            break;

        case 0x1D:  //DEC E
            // decrement e
            set_N(1, cpu);
            set_H_sub(cpu->e, 1, cpu);
            cpu->e -= 1;
            set_Z(cpu->e, cpu);
            break;

        case 0x1E:  //LD E, u8
            // Copy u8 into E
            cpu->e = cpu->memory[++cpu->pc];
            break;

        case 0x1F:  //RRA
            // Move lsb to carry
            // Move carry  to msb
            temp8 = (cpu->f & FLAG_C) ? 0x80 : 0x00; // bit in carry
            u8 = cpu->a & 0x01; // lsb
            cpu->a = (cpu->a >> 1) | temp8; // add carry to msb of a
            set_Z(0, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);  
            break;

        case 0x20:  //JR NZ, i8
        // Jump by i8 steps if Z flag is NOT set
            offset = (int8_t) cpu->memory[++cpu->pc];
            if(!(cpu->f & FLAG_Z))
                cpu->pc += offset;
            else
                ++cpu->pc;  
            break;

        case 0x21:  //LD HL, u16
            // Copy u16 into HL
            u16 = cpu->memory[++cpu->pc];
            u16 = u16 | (cpu->memory[++cpu->pc] << 8);
            cpu->hl = u16;
            break;

        case 0x22:  //LD HL+, A
            // Copy A into byte pointed by HL
            // Then Increment HL
            cpu->memory[cpu->hl] = cpu->a;
            cpu->hl += 1;
            break;

        case 0x23:  //INC HL
            // Don't confuse with 0x34 
            // Increments HL
            cpu->hl += 1;
            break;

        case 0x24:  //INC H 
            set_N(0, cpu);  
            set_H_add(cpu->h, 1, cpu);
            cpu->h += 1;
            set_Z(cpu->h, cpu);
            break;

        case 0x25:  //DEC H
            set_N(1, cpu);
            set_H_sub(cpu->h, 1, cpu);
            cpu->h -= 1;
            set_Z(cpu->h, cpu);
            break;

        case 0x26:  //LD H, u8
            // Copy u8 into H
            cpu->h = cpu->memory[++cpu->pc];
            break;

        case 0x27:  //DAA
        /*
        If the subtract flag N is set:

            - Initialize the adjustment to 0.
            - If the half-carry flag H is set, then add $6 to the adjustment.
            - If the carry flag is set, then add $60 to the adjustment.
            - Subtract the adjustment from A.

        If the subtract flag N is not set:

            - Initialize the adjustment to 0.
            - If the half-carry flag H is set or A & $F > $9, then add $6 to the adjustment.
            - If the carry flag is set or A > $99, then add $60 to the adjustment and set the carry flag.
            - Add the adjustment to A.
        */
            break;

        case 0x28:  //JR Z, i8
            // Jump by i8 steps if Z flag is set
            offset = (int8_t) cpu->memory[++cpu->pc];
            if(cpu->f & FLAG_Z)
                cpu->pc += offset;
            else
                ++cpu->pc;
            break;

        case 0x29:  //ADD HL, HL
            // Add HL to HL
            u16 = cpu->hl;
            cpu->hl = u16 + u16;
            set_N(0, cpu);
            set_H_add16(u16, u16,  cpu);
            set_C_add16(u16, u16, cpu); 
            break;

        case 0x2A:  //LD A, HL+
            // Copy byte pointed by HL into A
            // Increment HL
            cpu->a = cpu->memory[cpu->hl];
            cpu->hl += 1;
            break;

        case 0x2B:  //DEC HL
            // Decrement HL
            cpu->hl -= 1;
            break;

        case 0x2C:  //INC L 
            set_N(0, cpu);  
            set_H_add(cpu->l, 1, cpu);
            cpu->l += 1;
            set_Z(cpu->l, cpu);
            break;

        case 0x2D:  //DEC L
            set_N(1, cpu);
            set_H_sub(cpu->l, 1, cpu);
            cpu->l -= 1;
            set_Z(cpu->l, cpu);
            break;

        case 0x2E:  //LD L, u8
            // Copy u8 into L
            cpu->l = cpu->memory[++cpu->pc];
            break;

        case 0x2F:  //CPL
            // Ones complement of A
            set_N(1, cpu);
            set_H(1, cpu);
            cpu->a = ~cpu->a;
            break;

        case 0x30:  //JR NC, i8
            // Jump by i8 steps if C flag is NOT set
            offset = (int8_t) cpu->memory[++cpu->pc];
            if(!(cpu->f & FLAG_C))
                cpu->pc += offset;
            else
                ++cpu->pc;
            break;

        case 0x31:  //LD SP, u16
            // Load u16 into the SP
            u16 = cpu->memory[++cpu->pc];
            u16 = u16 | (cpu->memory[++cpu->pc] << 8);
            cpu->sp = u16;
            break;

        case 0x32:  //LD HL-, A
            // Load A into byte pointed by HL
            // Decrement HL
            cpu->memory[cpu->hl] = cpu->a;
            cpu->hl -= 1;
            break;

        case 0x33:  //INC SP
            // Increment stack pointer
            cpu->sp += 1;
            break;

        case 0x34:  //INC [HL]
            // Increment byte pointed by HL
            set_H_add(cpu->memory[cpu->hl], 1, cpu);
            cpu->memory[cpu->hl] += 1;
            set_Z(cpu->memory[cpu->hl], cpu);
            set_N(0, cpu);
            break;

        case 0x35:  //DEC [HL]
            //  Decrement byte pointed by HL
            set_H_sub(cpu->memory[cpu->hl], 1, cpu);
            cpu->memory[cpu->hl] -= 1;
            set_Z(cpu->memory[cpu->hl], cpu);
            set_N(1, cpu);
            break;

        case 0x36:  //LD [HL], u8
            // Load u8 into byte pointed by HL
            u8 =  cpu->memory[++cpu->pc];
            cpu->memory[cpu->hl] = u8;
            break;

        case 0x37:  //SCF
            // set C flag
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(1, cpu);
            break;

        case 0x38:  //JR C, i8
            // Jump by i8 if C is set
            offset = (int8_t) cpu->memory[++cpu->pc];
            if(cpu->f & FLAG_C)
                cpu->pc += offset;
            else
                ++cpu->pc;
            break;

        case 0x39:  //ADD HL, SP
            // Add value in sp to hl
            set_H_add16(cpu->hl, cpu->sp, cpu);
            set_C_add16(cpu->hl, cpu->sp, cpu);
            cpu->hl += cpu->sp;
            set_N(0, cpu);
            break;

        case 0x3A:  //LD A, [HL-]
            // Load byte pointed by HL
            // Decrement HL
            cpu->a = cpu->memory[cpu->hl];
            cpu->hl -= 1;
            break;

        case 0x3B:  //DEC SP
            // decrement SP
            cpu->sp -= 1;
            break;

        case 0x3C:  //INC A
            // Increment A
            set_N(0, cpu);  
            set_H_add(cpu->a, 1, cpu);
            cpu->a += 1;
            set_Z(cpu->a, cpu);
            break;

        case 0x3D:  //DEC A
            // Decrement A
            set_N(1, cpu);
            set_H_sub(cpu->a, 1, cpu);
            cpu->a -= 1;
            set_Z(cpu->a, cpu);
            break;

        case 0x3E:  //LD A, u8
            // Load u8 into A
            u8 = cpu->memory[++cpu->pc];
            cpu->a = u8;
            break;

        case 0x3F:  //CCF
            // Complement Carry Flag
            set_N(0, cpu);
            set_H(0, cpu);
            if(cpu->f & FLAG_C)
                set_C(0, cpu);
            else
                set_C(1, cpu);
            break;

        // The next 64 instructions are all 8bit Load - except 0x76 HALT 
        case 0x40:  //LD B, B
            cpu->b = cpu->b;
            break;

        case 0x41:  //LD B, C
            cpu->b = cpu->c;
            break;

        case 0x42:  //LD B, D
            cpu->b = cpu->d;
            break;

        case 0x43:  //LD B, E
            cpu->b = cpu->e;
            break;

        case 0x44:  //LD B, H
            cpu->b = cpu->h;
            break;

        case 0x45:  //LD B, L
            cpu->b = cpu->l;
            break;

        case 0x46:  //LD B, [HL]
            cpu->b = cpu->memory[cpu->hl];   
            break;

        case 0x47:  //LD B, A
            cpu->b = cpu->a;
            break;

        case 0x48:  //LD C, B
            cpu->c = cpu->b;
            break;

        case 0x49:  //LD C, C
            cpu->c = cpu->c;
            break;

        case 0x4A:  //LD C, D
            cpu->c = cpu->d;
            break;

        case 0x4B:  //LD C, E
            cpu->c = cpu->e;
            break;

        case 0x4C:  //LD C, H
            cpu->c = cpu->h;
            break;

        case 0x4D:  //LD C, L
            cpu->c = cpu->l;
            break;

        case 0x4E:  //LD C, HL
            cpu->c = cpu->memory[cpu->hl];
            break;

        case 0x4F:  //LD C, A
            cpu->c = cpu->a;
            break;

        case 0x50:  //LD D, B
            cpu->d = cpu->b;
            break;

        case 0x51:  //LD D, C
            cpu->d = cpu->c;
            break;

        case 0x52:  //LD D, D
            cpu->d = cpu->d;
            break;

        case 0x53:  //LD D, E
            cpu->d = cpu->e;
            break;

        case 0x54:  //LD D, H
            cpu->d = cpu->h;
            break;

        case 0x55:  //LD D, L
            cpu->d = cpu->l;
            break;

        case 0x56:  //LD D, HL
            cpu->d = cpu->memory[cpu->hl];
            break;

        case 0x57:  //LD D, A
            cpu->d = cpu->a;
            break;

        case 0x58:  //LD E, B
            cpu->e = cpu->b;
            break;

        case 0x59:  //LD E, C
            cpu->e = cpu->c;
            break;

        case 0x5A:  //LD E, D
            cpu->e = cpu->d;
            break;

        case 0x5B:  //LD E, E
            cpu->e = cpu->e;
            break;

        case 0x5C:  //LD E, H
            cpu->e = cpu->h;
            break;

        case 0x5D:  //LD E, L
            cpu->e = cpu->l;
            break;

        case 0x5E:  //LD E, HL
            cpu->e = cpu->memory[cpu->hl];
            break;

        case 0x5F:  //LD E, A
            cpu->e = cpu->a;
            break;

        case 0x60:  //LD H, B
            cpu->h = cpu->b;
            break;

        case 0x61:  //LD H, C
            cpu->h = cpu->c;
            break;

        case 0x62:  //LD H, D
            cpu->h = cpu->d;
            break;

        case 0x63:  //LD H, E
            cpu->h = cpu->e;
            break;

        case 0x64:  //LD H, H
            cpu->h = cpu->h;
            break;

        case 0x65:  //LD H, L
            cpu->h = cpu->l;
            break;

        case 0x66:  //LD H, HL
            cpu->h = cpu->memory[cpu->hl];
            break;

        case 0x67:  //LD H, A
            cpu->h = cpu->a;
            break;

        case 0x68:  //LD L, B
            cpu->l = cpu->b;
            break;

        case 0x69:  //LD L, C
            cpu->l = cpu->c;
            break;

        case 0x6A:  //LD L, D
            cpu->l = cpu->d;
            break;

        case 0x6B:  //LD L, E
            cpu->l = cpu->e;
            break;

        case 0x6C:  //LD L, H
            cpu->l = cpu->h;
            break;

        case 0x6D:  //LD L, L
            cpu->l = cpu->l;
            break;

        case 0x6E:  //LD L, HL
            cpu->l = cpu->memory[cpu->hl];
            break;

        case 0x6F:  //LD L, A
            cpu->l = cpu->a;
            break;

        case 0x70:  //LD HL, B
            cpu->memory[cpu->hl] = cpu->b;
            break;

        case 0x71:  //LD HL, C
            cpu->memory[cpu->hl] = cpu->c;
            break;

        case 0x72:  //LD HL, D
            cpu->memory[cpu->hl] = cpu->d;
            break;

        case 0x73:  //LD HL, E
            cpu->memory[cpu->hl] = cpu->e;
            break;

        case 0x74:  //LD HL, H
            cpu->memory[cpu->hl] = cpu->h;
            break;

        case 0x75:  //LD HL, L
            cpu->memory[cpu->hl] = cpu->l;
            break;

        case 0x76:  //HALT
            // HALT ------------------------- Recheck this one
            break;

        case 0x77:  //LD HL, A
            cpu->memory[cpu->hl] = cpu->a;
            break;

        case 0x78:  //LD A, B
            cpu->a = cpu->b;
            break;

        case 0x79:  //LD A, C
            cpu->a = cpu->c;
            break;

        case 0x7A:  //LD A, D
            cpu->a = cpu->d;
            break;

        case 0x7B:  //LD A, E
            cpu->a = cpu->e;
            break;

        case 0x7C:  //LD A, H
            cpu->a = cpu->h;
            break;

        case 0x7D:  //LD A, L
            cpu->a = cpu->l;
            break;

        case 0x7E:  //LD A, HL
            cpu->a = cpu->memory[cpu->hl];
            break;

        case 0x7F:  //LD A, A
            cpu->a = cpu->a;
            break;
        
        // The next 16 instructions are all ADD and ADC

        case 0x80:  //ADD A, B
            set_N(0, cpu);
            set_H_add(cpu->a, cpu->b, cpu);
            set_C_add(cpu->a, cpu->b, cpu);
            cpu->a += cpu->b;
            set_Z(cpu->a, cpu);
            break;

        case 0x81:  //ADD A, C
            set_N(0, cpu);
            set_H_add(cpu->a, cpu->c, cpu);
            set_C_add(cpu->a, cpu->c, cpu);
            cpu->a += cpu->c;
            set_Z(cpu->a, cpu);
            break;

        case 0x82:  //ADD A, D
            set_N(0, cpu);
            set_H_add(cpu->a, cpu->d, cpu);
            set_C_add(cpu->a, cpu->d, cpu);
            cpu->a += cpu->d;
            set_Z(cpu->a, cpu);
            break;

        case 0x83:  //ADD A, E
            set_N(0, cpu);
            set_H_add(cpu->a, cpu->e, cpu);
            set_C_add(cpu->a, cpu->e, cpu);
            cpu->a += cpu->e;
            set_Z(cpu->a, cpu);
            break;

        case 0x84:  //ADD A, H
            set_N(0, cpu);
            set_H_add(cpu->a, cpu->h, cpu);
            set_C_add(cpu->a, cpu->h, cpu);
            cpu->a += cpu->h;
            set_Z(cpu->a, cpu);
            break;

        case 0x85:  //ADD A, L
            set_N(0, cpu);
            set_H_add(cpu->a, cpu->l, cpu);
            set_C_add(cpu->a, cpu->l, cpu);
            cpu->a += cpu->l;
            set_Z(cpu->a, cpu);
            break;

        case 0x86:  //ADD A, [HL]
            set_N(0, cpu);
            set_H_add(cpu->a, cpu->memory[cpu->hl], cpu);
            set_C_add(cpu->a, cpu->memory[cpu->hl], cpu);
            cpu->a += cpu->memory[cpu->hl];
            set_Z(cpu->a, cpu);
            break;

        case 0x87:  //ADD A, A
            set_N(0, cpu);
            set_H_add(cpu->a, cpu->a, cpu);
            set_C_add(cpu->a, cpu->a, cpu);
            cpu->a += cpu->a;
            set_Z(cpu->a, cpu);
            break;

        case 0x88:  //ADC A, B
            u8 = cpu->b + (cpu->f & FLAG_C);
            set_N(0, cpu);
            set_H_add(cpu->a, u8, cpu);
            set_C_add(cpu->a, u8, cpu);
            cpu->a += u8;
            set_Z(cpu->a, cpu);
            break;

        case 0x89:  //ADC A, C
            u8 = cpu->c + (cpu->f & FLAG_C);
            set_N(0, cpu);
            set_H_add(cpu->a, u8, cpu);
            set_C_add(cpu->a, u8, cpu);
            cpu->a += u8;
            set_Z(cpu->a, cpu);
            break;

        case 0x8A:  //ADC A, D
            u8 = cpu->d + (cpu->f & FLAG_C);
            set_N(0, cpu);
            set_H_add(cpu->a, u8, cpu);
            set_C_add(cpu->a, u8, cpu);
            cpu->a += u8;
            set_Z(cpu->a, cpu);
            break;

        case 0x8B:  //ADC A, E
            u8 = cpu->e + (cpu->f & FLAG_C);
            set_N(0, cpu);
            set_H_add(cpu->a, u8, cpu);
            set_C_add(cpu->a, u8, cpu);
            cpu->a += u8;
            set_Z(cpu->a, cpu);
            break;

        case 0x8C:  //ADC A, H
            u8 = cpu->h + (cpu->f & FLAG_C);
            set_N(0, cpu);
            set_H_add(cpu->a, u8, cpu);
            set_C_add(cpu->a, u8, cpu);
            cpu->a += u8;
            set_Z(cpu->a, cpu);
            break;

        case 0x8D:  //ADC A, L
            u8 = cpu->l + (cpu->f & FLAG_C);
            set_N(0, cpu);
            set_H_add(cpu->a, u8, cpu);
            set_C_add(cpu->a, u8, cpu);
            cpu->a += u8;
            set_Z(cpu->a, cpu);
            break;

        case 0x8E:  //ADC A, [HL]
            u8 = cpu->memory[cpu->hl] + (cpu->f & FLAG_C);
            set_N(0, cpu);
            set_H_add(cpu->a, u8, cpu);
            set_C_add(cpu->a, u8, cpu);
            cpu->a += u8;
            set_Z(cpu->a, cpu);
            break;

        case 0x8F:  //ADC A, A
            u8 = cpu->a + (cpu->f & FLAG_C);
            set_N(0, cpu);
            set_H_add(cpu->a, u8, cpu);
            set_C_add(cpu->a, u8, cpu);
            cpu->a += u8;
            set_Z(cpu->a, cpu);
            break;

        case 0x90:  //SUB B
            set_N(1, cpu);
            set_H_sub(cpu->a, cpu->b, cpu);
            set_C_sub(cpu->a, cpu->b, cpu);
            cpu->a -= cpu->b;
            set_Z(cpu->a, cpu);
            break;

        case 0x91:  //SUB C
            set_N(1, cpu);
            set_H_sub(cpu->a, cpu->c, cpu);
            set_C_sub(cpu->a, cpu->c, cpu);
            cpu->a -= cpu->c;
            set_Z(cpu->a, cpu);
            break;

        case 0x92:  //SUB D
            set_N(1, cpu);
            set_H_sub(cpu->a, cpu->d, cpu);
            set_C_sub(cpu->a, cpu->d, cpu);
            cpu->a -= cpu->d;
            set_Z(cpu->a, cpu);
            break;

        case 0x93:  //SUB E
            set_N(1, cpu);
            set_H_sub(cpu->a, cpu->e, cpu);
            set_C_sub(cpu->a, cpu->e, cpu);
            cpu->a -= cpu->e;
            set_Z(cpu->a, cpu);
            break;

        case 0x94:  //SUB H
            set_N(1, cpu);
            set_H_sub(cpu->a, cpu->h, cpu);
            set_C_sub(cpu->a, cpu->h, cpu);
            cpu->a -= cpu->h;
            set_Z(cpu->a, cpu);
            break;

        case 0x95:  //SUB L
            set_N(1, cpu);
            set_H_sub(cpu->a, cpu->l, cpu);
            set_C_sub(cpu->a, cpu->l, cpu);
            cpu->a -= cpu->l;
            set_Z(cpu->a, cpu);
            break;

        case 0x96:  //SUB [HL]
            set_N(1, cpu);
            set_H_sub(cpu->a, cpu->memory[cpu->hl], cpu);
            set_C_sub(cpu->a, cpu->memory[cpu->hl], cpu);
            cpu->a -= cpu->memory[cpu->hl];
            set_Z(cpu->a, cpu);
            break;

        case 0x97:  //SUB A
            set_N(1, cpu);
            set_H_sub(cpu->a, cpu->a, cpu);
            set_C_sub(cpu->a, cpu->a, cpu);
            cpu->a -= cpu->a;
            set_Z(cpu->a, cpu);
            break;

        case 0x98:  //SBC A, B
            u8 = cpu->b + (cpu->f & FLAG_C);
            set_N(1, cpu);
            set_H_sub(cpu->a, u8, cpu);
            set_C_sbc(cpu->a, u8, cpu->f & FLAG_C, cpu);
            cpu->a -= u8;
            set_Z(cpu->a, cpu);
            break;

        case 0x99:  //SBC A, C
            u8 = cpu->c + (cpu->f & FLAG_C);
            set_N(1, cpu);
            set_H_sub(cpu->a, u8, cpu);
            set_C_sbc(cpu->a, u8, cpu->f & FLAG_C, cpu);
            cpu->a -= u8;
            set_Z(cpu->a, cpu);
            break;

        case 0x9A:  //SBC A, D
            u8 = cpu->d + (cpu->f & FLAG_C);
            set_N(1, cpu);
            set_H_sub(cpu->a, u8, cpu);
            set_C_sbc(cpu->a, u8, cpu->f & FLAG_C, cpu);
            cpu->a -= u8;
            set_Z(cpu->a, cpu);
            break;

        case 0x9B:  //SBC A, E
            u8 = cpu->e + (cpu->f & FLAG_C);
            set_N(1, cpu);
            set_H_sub(cpu->a, u8, cpu);
            set_C_sbc(cpu->a, u8, cpu->f & FLAG_C, cpu);
            cpu->a -= u8;
            set_Z(cpu->a, cpu);
            break;

        case 0x9C:  //SBC A, H
            u8 = cpu->h + (cpu->f & FLAG_C);
            set_N(1, cpu);
            set_H_sub(cpu->a, u8, cpu);
            set_C_sbc(cpu->a, u8, cpu->f & FLAG_C, cpu);
            cpu->a -= u8;
            set_Z(cpu->a, cpu);
            break;

        case 0x9D:  //SBC A, L
            u8 = cpu->l + (cpu->f & FLAG_C);
            set_N(1, cpu);
            set_H_sub(cpu->a, u8, cpu);
            set_C_sbc(cpu->a, u8, cpu->f & FLAG_C, cpu);
            cpu->a -= u8;
            set_Z(cpu->a, cpu);
            break;

        case 0x9E:  //SBC A, HL
            u8 = cpu->memory[cpu->hl] + (cpu->f & FLAG_C);
            set_N(1, cpu);
            set_H_sub(cpu->a, u8, cpu);
            set_C_sbc(cpu->a, u8, cpu->f & FLAG_C, cpu);
            cpu->a -= u8;
            set_Z(cpu->a, cpu);
            break;

        case 0x9F:  //SBC A, A
            u8 = cpu->a + (cpu->f & FLAG_C);
            set_N(1, cpu);
            set_H_sub(cpu->a, u8, cpu);
            set_C_sbc(cpu->a, u8, cpu->f & FLAG_C, cpu);
            cpu->a -= u8;
            set_Z(cpu->a, cpu);
            break;

        case 0xA0:  //AND B
            cpu->a = (cpu->a & cpu->b);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            break;

        case 0xA1:  //AND C
            cpu->a = (cpu->a & cpu->c);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            break;

        case 0xA2:  //AND D
            cpu->a = (cpu->a & cpu->d);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            break;

        case 0xA3:  //AND E
            cpu->a = (cpu->a & cpu->e);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            break;

        case 0xA4:  //AND H
            cpu->a = (cpu->a & cpu->h);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            break;

        case 0xA5:  //AND L
            cpu->a = (cpu->a & cpu->l);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            break;

        case 0xA6:  //AND [HL]
            cpu->a = (cpu->a & cpu->memory[cpu->hl]);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            break;

        case 0xA7:  //AND A
            cpu->a = (cpu->a & cpu->a);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            break;

        case 0xA8:  //XOR B
            cpu->a = (cpu->a ^ cpu->b);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0xA9:  //XOR C
            cpu->a = (cpu->a ^ cpu->c);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0xAA:  //XOR D
            cpu->a = (cpu->a ^ cpu->d);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0xAB:  //XOR E
            cpu->a = (cpu->a ^ cpu->e);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0xAC:  //XOR H
            cpu->a = (cpu->a ^ cpu->h);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0xAD:  //XOR L
            cpu->a = (cpu->a ^ cpu->l);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0xAE:  //XOR HL
            cpu->a = (cpu->a ^ cpu->memory[cpu->hl]);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0xAF:  //XOR A
            cpu->a = (cpu->a ^ cpu->a);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0xB0:  //OR B
            cpu->a = (cpu->a | cpu->b);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0xB1:  //OR C
            cpu->a = (cpu->a | cpu->c);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0xB2:  //OR D
            cpu->a = (cpu->a | cpu->d);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0xB3:  //OR E
            cpu->a = (cpu->a | cpu->e);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0xB4:  //OR H
            cpu->a = (cpu->a | cpu->h);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0xB5:  //OR L
            cpu->a = (cpu->a | cpu->l);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

    case 0xB6:  //OR [HL]
            cpu->a = (cpu->a | cpu->memory[cpu->hl]);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0xB7:  //OR A
            cpu->a = (cpu->a | cpu->a);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0xB8:  //CP B
            temp8 = cpu->a - cpu->b;
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(cpu->a, cpu->b, cpu);
            set_C_sub(cpu->a, cpu->b, cpu);
            break;

        case 0xB9:  //CP C
            temp8 = cpu->a - cpu->c;
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(cpu->a, cpu->c, cpu);
            set_C_sub(cpu->a, cpu->c, cpu);
            break;

        case 0xBA:  //CP D
            temp8 = cpu->a - cpu->d;
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(cpu->a, cpu->d, cpu);
            set_C_sub(cpu->a, cpu->d, cpu);
            break;

        case 0xBB:  //CP E
            temp8 = cpu->a - cpu->e;
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(cpu->a, cpu->e, cpu);
            set_C_sub(cpu->a, cpu->e, cpu);
            break;

        case 0xBC:  //CP H
            temp8 = cpu->a - cpu->h;
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(cpu->a, cpu->h, cpu);
            set_C_sub(cpu->a, cpu->h, cpu);
            break;

        case 0xBD:  //CP L
            temp8 = cpu->a - cpu->l;
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(cpu->a, cpu->l, cpu);
            set_C_sub(cpu->a, cpu->l, cpu);
            break;

        case 0xBE:  //CP HL
            temp8 = cpu->a - cpu->memory[cpu->hl];
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(cpu->a, cpu->memory[cpu->hl], cpu);
            set_C_sub(cpu->a, cpu->memory[cpu->hl], cpu);
            break;

        case 0xBF:  //CP A
            temp8 = cpu->a - cpu->a;
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(cpu->a, cpu->a, cpu);
            set_C_sub(cpu->a, cpu->a, cpu);
            break;

        case 0xC0:  //RET NZ
            if (!(cpu->f & FLAG_Z)) {
                uint8_t low = cpu->memory[cpu->sp++];
                uint8_t high = cpu->memory[cpu->sp++];
                cpu->pc = (high << 8) | low;
                cycles = 20;
            } else {
                cycles = 8;
            }
            break;

        case 0xC1:  //POP BC
            break;

        case 0xC2:  //JP NZ, u16
        // Jump to u16 if Z is not set
            if(!(cpu->f & FLAG_Z))
                cpu->pc = cpu->memory[++cpu->pc];
            else
                ++cpu->pc;
            break;

        case 0xC3:  //JP u16
            // Jump to u16
            cpu->pc = cpu->memory[++cpu->pc];
            break;

        case 0xC4:  //CALL NZ, u16
            break;

        case 0xC5:  //PUSH BC
            break;

        case 0xC6:  //ADD A, u8
            set_N(0, cpu);
            u8 = cpu->memory[++cpu->pc];
            set_H_add(cpu->a, u8, cpu);
            set_C_add(cpu->a, u8, cpu);
            cpu->a += u8;
            set_Z(cpu->a, cpu);
            break;

        case 0xC7:  //RST 00H
            break;

        case 0xC8:  //RET Z
            break;

        case 0xC9:  //RET
            break;

        case 0xCA:  //JP Z, u16
            break;

        case 0xCB:  //Prefix CB
            run_pref_inst(cpu->memory[++cpu->pc]);
            break;

        case 0xCC:  //CALL Z, u16
            break;

        case 0xCD:  //CALL u16
            break;

        case 0xCE:  //ADC A, u8
            u8 = cpu->memory[++cpu->pc] + (cpu->f & FLAG_C);
            set_N(0, cpu);
            set_H_add(cpu->a, u8, cpu);
            set_C_add(cpu->a, u8, cpu);
            cpu->a += u8;
            set_Z(cpu->a, cpu);
            break;

        case 0xCF:  //RST 08H
            break;

        case 0xD0:  //RET NC
            break;

        case 0xD1:  //POP DE
            break;

        case 0xD2:  //JP NC, u16
            // Jump to u16 if C is  NOT set
            if(!(cpu->f & FLAG_C))
                cpu->pc = cpu->memory[++cpu->pc];
            else
                ++cpu->pc;
            break;

        case 0xD3:  //HOLE
            break;

        case 0xD4:  //CALL NC, u16
            break;

        case 0xD5:  //PUSH DE
            break;

        case 0xD6:  //SUB A, u8
            // Subtract u8 from A
            set_N(1, cpu);
            u8 = cpu->memory[++cpu->pc];
            set_H_sub(cpu->a, u8, cpu);
            set_C_sub(cpu->a, u8, cpu);
            cpu->a -= u8;
            set_Z(cpu->a, cpu);
            break;

        case 0xD7:  //RST 10H
            break;

        case 0xD8:  //RET C
            break;

        case 0xD9:  //RETI
            break;

        case 0xDA:  //JP C, u16
            // Jump to u16 if C is set
            if(cpu->f & FLAG_C)
                cpu->pc = cpu->memory[++cpu->pc];
            else
                ++cpu->pc;
            break;

        case 0xDB:  //HOLE
            break;

        case 0xDC:  //CALL C, u16
            break;

        case 0xDD:  //HOLE
            break;

        case 0xDE:  //SBC A, u8
            u8 = cpu->memory[++cpu->pc] + (cpu->f & FLAG_C);
            set_N(1, cpu);
            set_H_sub(cpu->a, u8, cpu);
            set_C_sbc(cpu->a, u8, cpu->f & FLAG_C, cpu);
            cpu->a -= u8;
            set_Z(cpu->a, cpu);
            break;

        case 0xDF:  //RST 18H
            break;

        case 0xE0:  //LDH u8, A
            break;

        case 0xE1:  //POP HL
            break;

        case 0xE2:  //LD C, A
            break;

        case 0xE3:  //HOLE
            break;

        case 0xE4:  //HOLE
            break;

        case 0xE5:  //PUSH HL
            break;

        case 0xE6:  //AND A, u8
            cpu->a = (cpu->a & cpu->memory[++cpu->pc]);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            break;

        case 0xE7:  //RST 20H
            break;

        case 0xE8:  //ADD SP, i8
            // 16bit ADD - signed i8 to sp

            break;

        case 0xE9:  //JP HL
            // Jump to HL
                cpu->pc = cpu->memory[cpu->hl];
            break;

        case 0xEA:  //LD u16, A
            break;

        case 0xEB:  //HOLE
            break;

        case 0xEC:  //HOLE
            break;

        case 0xED:  //HOLE
            break;

        case 0xEE:  //XOR u8
            cpu->a = (cpu->a ^ cpu->memory[++cpu->pc]);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0xEF:  //RST 28H
            break;

        case 0xF0:  //LDH A, u8
            break;

        case 0xF1:  //POP AF
            break;

        case 0xF2:  //LD A, C
            break;

        case 0xF3:  //DI
            break;

        case 0xF4:  //HOLE
            break;

        case 0xF5:  //PUSH AF
            break;

        case 0xF6:  //OR A, u8
            cpu->a = (cpu->a | cpu->memory[++cpu->pc]);
            set_Z(cpu->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            break;

        case 0xF7:  //RST 30H
            break;

        case 0xF8:  //LD HL, SP+i8
            break;

        case 0xF9:  //LD SP, HL
            break;

        case 0xFA:  //LD A, u16
            break;

        case 0xFB:  //EI
            break;

        case 0xFC:  //HOLE
            break;

        case 0xFD:  //HOLE
            break;

        case 0xFE:  //CP u8
            u8 = cpu->memory[++cpu->pc];
            temp8 = cpu->a - u8;
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(cpu->a, u8, cpu);
            set_C_sub(cpu->a, u8, cpu);
            break;

        case 0xFF:  //RST 38H
            break;
    }
}