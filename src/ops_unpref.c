#include "cpu.h"
#include "emu.h"
// TODO- Check set_Z implementation. You need to pass 1 to unset, which is reeally unintuitive

void run_inst(uint8_t opcode, CPU *cpu){
    // temporary variables
    uint16_t u16;
    uint8_t u8;
    uint8_t temp8;
    int8_t offset;
    Registers *reg = &cpu->regs;

    switch(opcode){
        case 0x00:  //NOP
            cpu->pc += 1;
            break;

        case 0x01:  //LD BC, u16 
            // write u16 into BC
            reg->bc = read16(cpu, cpu->pc + 1);
            cpu->pc += 3; 
            cpu->cycles += 3; 
            break;

        case 0x02:  //LD BC, A 
            //write from A to byte pointed by BC
            write8(cpu, reg->bc, reg->a);
            cpu->pc += 1;
            cpu->cycles += 2;    
            break;

        case 0x03:  //INC BC
            //Increment value of BC by 1
            reg->bc += 1;
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x04:  //INC B
            //Increment value of B by 1
            set_H_add(reg->b, 1, cpu);
            set_N(0,cpu);
            reg->b += 1;
            set_Z(reg->b, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x05:  //DEC B
            // Decrease value of B by 1
            set_H_sub(reg->b, 1, cpu);
            reg->b -= 1;
            // Change Z and set N flag
            set_Z(reg->b, cpu);
            set_N(1, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x06:  //LD B, u8
            // Write u8 into B
            reg->b = read8(cpu, cpu->pc+1);
            cpu->pc += 2;
            cpu->cycles += 2;
            break;
            
        case 0x07:  //RLCA
            // Rotate Left A
            u8 = (reg->a >> 7) & 1; // recording the msb
            set_Z(1, cpu);   // passing 1 basically unsets it 
            set_N(0, cpu); 
            set_H(0, cpu);
            set_C(u8, cpu);
            reg->a = reg->a << 1;
            reg->a = reg->a | u8;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x08:  //LD u16, SP     
            // Copy SP & $FF at address u16 and SP >> 8 at address u16 + 1.
            u16 = read16(cpu, cpu->pc+1);
            write16(cpu, u16, cpu->sp);
            cpu->pc += 3;
            cpu->cycles += 5;
            break;

        case 0x09:  //ADD HL, BC
            // Add value of BC into HL
            set_N(0, cpu);
            set_H_add16(reg->hl, reg->bc, cpu);
            set_C_add16(reg->hl, reg->bc, cpu);
            reg->hl += reg->bc;
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x0A:  //LD A, BC
            // Write Byte pointed by BC into A
            reg->a = read8(cpu, reg->bc);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;
        
        case 0x0B:  //DEC BC
            // Decrement value in BC by 1
            reg->bc -= 1;
            cpu->pc += 1;
            cpu->cycles += 2;
            break;  

        case 0x0C:  //INC C
            // Increment value in C by 1
            set_N(0, cpu);
            set_H_add(reg->c, 1,cpu);
            reg->c += 1;
            set_Z(reg->c, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;  

        case 0x0D:  //DEC C
            // Decrement value in C by 1
            set_N(1, cpu);
            set_H_sub(reg->c, 1, cpu);
            reg->c -= 1;
            set_Z(reg->c, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;  

        case 0x0E:  //LD C, u8
            // Write u8 into C
            reg->c = read8(cpu, cpu->pc+1);
            cpu->pc += 2;
            cpu->cycles += 2;
            break;  

        case 0x0F:  //RRCA
            // Rotate Register A right
            set_Z(1, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            u8 = reg->a & 0x01; // recording lsb
            set_C(u8, cpu);
            reg->a = (reg->a >> 1) | (u8 << 7);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;
            
        case 0x10:  //STOP
            u8 = read8(cpu, cpu->pc + 1);
            cpu->stopped = true;
            cpu->pc += 2;
            // STOP switches CPU to double-speed mode on CGB, does nothing on DMG
            cpu->cycles += 1;
            break;

        case 0x11:  //LD DE, u16
            // Load u16 into DE
            reg->de = read16(cpu, cpu->pc+1);
            cpu->pc += 3;
            cpu->cycles += 3;
            break;

        case 0x12:  //LD DE, A
            // Copy the value in register A into the byte pointed to by DE
            write8(cpu,reg->de,reg->a);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x13:  //INC DE
            // Increment value in DE by 1
            reg->de += 1;
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x14:  //INC D
            // Incremenrt value in D by 1
            set_N(0, cpu);  
            set_H_add(reg->d, 1, cpu);
            reg->d += 1;
            set_Z(reg->d, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x15:  //DEC D
            // Decrease value in D by 1
            set_Z(reg->d - 1, cpu);
            set_N(1, cpu);  
            set_H_sub(reg->d, 1, cpu);
            reg->d -= 1;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;
        
        case 0x16:  //LD D, u8
            // Load u8 into D
            reg->d = read8(cpu, cpu->pc+1);
            cpu->pc += 2;
            cpu->cycles += 2;
            break;
        
        case 0x17:  //RLA
            // Rotate A left through carry
            set_Z(1, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            // put msb into carry
            // then put carry into lsb
            u8 = (reg->a >> 7) & 1; 
            temp8 = (reg->f & FLAG_C)? 1 : 0;
            set_C(u8, cpu);
            reg->a = reg->a << 1;
            reg->a = reg->a | temp8;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x18:  //JR i8
            // Jump relative by i8 steps in pc
            offset = (int8_t) read8(cpu, cpu->pc+1);
            cpu->pc += 2;
            cpu->pc += offset;
            cpu->cycles += 3; 
            break;
        
        case 0x19:  //ADD HL, DE
            // Add value of DE to HL
            set_N(0, cpu);
            set_H_add16(reg->hl, reg->de, cpu);
            set_C_add16(reg->hl, reg->de, cpu);
            reg->hl += reg->de;
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x1A:  //LD A, DE
            // Copy Byte pointed by DE into A
            reg->a = read8(cpu, reg->de);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;
        
        case 0x1B:  //DEC DE
            // decrement de
            reg->de -= 1;
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x1C:  //INC E
            // Increment e 
            set_N(0, cpu);  
            set_H_add(reg->e, 1, cpu);
            reg->e += 1;
            set_Z(reg->e, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x1D:  //DEC E
            // decrement e
            set_N(1, cpu);
            set_H_sub(reg->e, 1, cpu);
            reg->e -= 1;
            set_Z(reg->e, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x1E:  //LD E, u8
            // Copy u8 into E
            reg->e = read8(cpu, cpu->pc+1);
            cpu->pc += 2;
            cpu->cycles += 2;
            break;

        case 0x1F:  //RRA
            // Move lsb to carry
            // Move carry  to msb
            temp8 = (reg->f & FLAG_C) ? 0x80 : 0x00; // bit in carry
            u8 = reg->a & 0x01; // lsb
            reg->a = (reg->a >> 1) | temp8; // add carry to msb of a
            set_Z(0, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(u8, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;  
            break;

        case 0x20:  //JR NZ, i8
        // Jump by i8 steps if Z flag is NOT set
            offset = (int8_t)read8(cpu, cpu->pc+1); 
            cpu->pc += 2;
            if(!(reg->f & FLAG_Z)){
                cpu->pc += offset;  
                cpu->cycles += 3;
            }
            else{
                cpu->cycles += 2;
            }  
            break;

        case 0x21:  //LD HL, u16
            // Copy u16 into HL
            reg->hl = read16(cpu, cpu->pc+1);
            cpu->pc += 3;
            cpu->cycles += 3;
            break;

        case 0x22:  //LD HL+, A
            // Copy A into byte pointed by HL
            // Then Increment HL
            write8(cpu, reg->hl, reg->a);
            reg->hl += 1;
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x23:  //INC HL
            // Don't confuse with 0x34 
            // Increments HL
            reg->hl += 1;
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x24:  //INC H 
            set_N(0, cpu);  
            set_H_add(reg->h, 1, cpu);
            reg->h += 1;
            set_Z(reg->h, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x25:  //DEC H
            set_N(1, cpu);
            set_H_sub(reg->h, 1, cpu);
            reg->h -= 1;
            set_Z(reg->h, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x26:  //LD H, u8
            // Copy u8 into H
            reg->h = read8(cpu, cpu->pc+1);
            cpu->pc += 2;
            cpu->cycles += 2;
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
            uint8_t adjustment = 0;
            bool carry = (reg->f & FLAG_C) != 0;;

            if (reg->f & FLAG_N) {
                if (reg->f & FLAG_H)
                    adjustment |= 0x06;
                if (reg->f & FLAG_C)
                    adjustment |= 0x60;
                    
                reg->a -= adjustment;
            } else {
                if (reg->f & FLAG_H || (reg->a & 0x0F) > 0x09)
                    adjustment |= 0x06;
                if (reg->f & FLAG_C || reg->a > 0x99){
                    adjustment |= 0x60;
                    carry = true;  
                }
                reg->a += adjustment;
            }

            set_Z(reg->a, cpu);
            set_H(0, cpu);
            set_C(carry, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x28:  //JR Z, i8
            // Jump by i8 steps if Z flag is set
            offset = (int8_t) read8(cpu, cpu->pc+1);
            cpu->pc += 2;
            if(reg->f & FLAG_Z){
                cpu->pc += offset;
                cpu->cycles += 3;
            }
            else{
                cpu->cycles += 2;
            }
            break;

        case 0x29:  //ADD HL, HL
            // Add HL to HL
            u16 = reg->hl;
            reg->hl = u16 + u16;
            set_N(0, cpu);
            set_H_add16(u16, u16,  cpu);
            set_C_add16(u16, u16, cpu);
            cpu->pc += 1;
            cpu->cycles += 2; 
            break;

        case 0x2A:  //LD A, HL+
            // Copy byte pointed by HL into A
            // Increment HL
            reg->a = read8(cpu, reg->hl);
            reg->hl += 1;
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x2B:  //DEC HL
            // Decrement HL
            reg->hl -= 1;
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x2C:  //INC L 
            set_N(0, cpu);  
            set_H_add(reg->l, 1, cpu);
            reg->l += 1;
            set_Z(reg->l, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x2D:  //DEC L
            set_N(1, cpu);
            set_H_sub(reg->l, 1, cpu);
            reg->l -= 1;
            set_Z(reg->l, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x2E:  //LD L, u8
            // Copy u8 into L
            reg->l = read8(cpu, cpu->pc+1);
            cpu->pc += 2;
            cpu->cycles += 2;
            break;

        case 0x2F:  //CPL
            // Ones complement of A
            set_N(1, cpu);
            set_H(1, cpu);
            reg->a = ~reg->a;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x30:  //JR NC, i8
            // Jump by i8 steps if C flag is NOT set
            offset = (int8_t) read8(cpu, cpu->pc+1);
            cpu->pc += 2;
            if(!(reg->f & FLAG_C)){
                cpu->pc += offset;
                cpu->cycles += 3;
            }
            else{
                cpu->cycles += 2;
            }
            break;

        case 0x31:  //LD SP, u16
            // Load u16 into the SP
            cpu->sp = read16(cpu, cpu->pc+1);
            cpu->pc += 3;   
            cpu->cycles += 3; 
            break;

        case 0x32:  //LD HL-, A
            // Load A into byte pointed by HL
            // Decrement HL
            write8(cpu, reg->hl, reg->a);
            reg->hl -= 1;
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x33:  //INC SP
            // Increment stack pointer
            cpu->sp += 1;
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x34:  //INC [HL]
            // Increment byte pointed by HL
            u8 = read8(cpu,reg->hl);
            set_H_add(u8, 1, cpu);
            write8(cpu, reg->hl, u8+1);
            set_Z(u8+1, cpu);
            set_N(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 3;
            break;

        case 0x35:  //DEC [HL]
            //  Decrement byte pointed by HL    
            u8 = read8(cpu,reg->hl);
            set_H_sub(u8, 1, cpu);
            write8(cpu, reg->hl, u8-1);
            set_Z(u8-1, cpu);
            set_N(1, cpu);
            cpu->pc += 1;
            cpu->cycles += 3;
            break;

        case 0x36:  //LD [HL], u8
            // Load u8 into byte pointed by HL
            u8 =  read8(cpu, cpu->pc+1);
            write8(cpu, reg->hl, u8);
            cpu->pc += 2;
            cpu->cycles += 3;
            break;

        case 0x37:  //SCF
            // set C flag
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(1, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x38:  //JR C, i8
            // Jump by i8 if C is set
            offset = (int8_t) read8(cpu, cpu->pc+1);
            cpu->pc += 2;
            if(reg->f & FLAG_C){
                cpu->pc += offset;
                cpu->cycles += 3;
            }
            else{
                cpu->cycles += 2;
            }
            break;

        case 0x39:  //ADD HL, SP
            // Add value in sp to hl
            set_H_add16(reg->hl, cpu->sp, cpu);
            set_C_add16(reg->hl, cpu->sp, cpu);
            reg->hl += cpu->sp;
            set_N(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x3A:  //LD A, [HL-]
            // Load byte pointed by HL
            // Decrement HL
            reg->a = read8(cpu, reg->hl);
            reg->hl -= 1;
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x3B:  //DEC SP
            // decrement SP
            cpu->sp -= 1;
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x3C:  //INC A
            // Increment A
            set_N(0, cpu);  
            set_H_add(reg->a, 1, cpu);
            reg->a += 1;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x3D:  //DEC A
            // Decrement A
            set_N(1, cpu);
            set_H_sub(reg->a, 1, cpu);
            reg->a -= 1;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x3E:  //LD A, u8
            // Load u8 into A
            u8 = read8(cpu, cpu->pc+1);
            reg->a = u8;
            cpu->pc += 2;
            cpu->cycles += 2;
            break;

        case 0x3F:  //CCF
            // Complement Carry Flag
            set_N(0, cpu);
            set_H(0, cpu);
            if(reg->f & FLAG_C)
                set_C(0, cpu);
            else
                set_C(1, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        // The next 64 instructions are all 8bit Load - except 0x76 HALT 
        case 0x40:  //LD B, B
            reg->b = reg->b;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x41:  //LD B, C
            reg->b = reg->c;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x42:  //LD B, D
            reg->b = reg->d;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x43:  //LD B, E
            reg->b = reg->e;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x44:  //LD B, H
            reg->b = reg->h;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x45:  //LD B, L
            reg->b = reg->l;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x46:  //LD B, [HL]
            reg->b = read8(cpu, reg->hl);
            cpu->pc += 1;
            cpu->cycles += 2;   
            break;

        case 0x47:  //LD B, A
            reg->b = reg->a;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x48:  //LD C, B
            reg->c = reg->b;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x49:  //LD C, C
            reg->c = reg->c;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x4A:  //LD C, D
            reg->c = reg->d;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x4B:  //LD C, E
            reg->c = reg->e;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x4C:  //LD C, H
            reg->c = reg->h;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x4D:  //LD C, L
            reg->c = reg->l;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x4E:  //LD C, HL
            reg->c = read8(cpu, reg->hl);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x4F:  //LD C, A
            reg->c = reg->a;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x50:  //LD D, B
            reg->d = reg->b;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x51:  //LD D, C
            reg->d = reg->c;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x52:  //LD D, D
            reg->d = reg->d;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x53:  //LD D, E
            reg->d = reg->e;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x54:  //LD D, H
            reg->d = reg->h;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x55:  //LD D, L
            reg->d = reg->l;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x56:  //LD D, HL
            reg->d = read8(cpu, reg->hl);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x57:  //LD D, A
            reg->d = reg->a;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x58:  //LD E, B
            reg->e = reg->b;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x59:  //LD E, C
            reg->e = reg->c;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x5A:  //LD E, D
            reg->e = reg->d;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x5B:  //LD E, E
            reg->e = reg->e;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x5C:  //LD E, H
            reg->e = reg->h;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x5D:  //LD E, L
            reg->e = reg->l;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x5E:  //LD E, HL
            reg->e = read8(cpu, reg->hl);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x5F:  //LD E, A
            reg->e = reg->a;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x60:  //LD H, B
            reg->h = reg->b;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x61:  //LD H, C
            reg->h = reg->c;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x62:  //LD H, D
            reg->h = reg->d;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x63:  //LD H, E
            reg->h = reg->e;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x64:  //LD H, H
            reg->h = reg->h;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x65:  //LD H, L
            reg->h = reg->l;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x66:  //LD H, HL
            reg->h = read8(cpu, reg->hl);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x67:  //LD H, A
            reg->h = reg->a;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x68:  //LD L, B
            reg->l = reg->b;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x69:  //LD L, C
            reg->l = reg->c;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x6A:  //LD L, D
            reg->l = reg->d;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x6B:  //LD L, E
            reg->l = reg->e;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x6C:  //LD L, H
            reg->l = reg->h;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x6D:  //LD L, L
            reg->l = reg->l;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x6E:  //LD L, HL
            reg->l = read8(cpu, reg->hl);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x6F:  //LD L, A
            reg->l = reg->a;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x70:  //LD HL, B
            write8(cpu, reg->hl, reg->b);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x71:  //LD HL, C
            write8(cpu, reg->hl, reg->c);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x72:  //LD HL, D
            write8(cpu, reg->hl, reg->d);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x73:  //LD HL, E
            write8(cpu, reg->hl, reg->e);
            cpu->cycles += 2;
            cpu->pc += 1;
            break;

        case 0x74:  //LD HL, H
            write8(cpu, reg->hl, reg->h);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x75:  //LD HL, L
            write8(cpu, reg->hl, reg->l);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x76:  //HALT
            cpu->halted= true;
            break;

        case 0x77:  //LD HL, A
            write8(cpu, reg->hl, reg->a);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x78:  //LD A, B
            reg->a = reg->b;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x79:  //LD A, C
            reg->a = reg->c;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x7A:  //LD A, D
            reg->a = reg->d;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x7B:  //LD A, E
            reg->a = reg->e;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x7C:  //LD A, H
            reg->a = reg->h;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x7D:  //LD A, L
            reg->a = reg->l;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x7E:  //LD A, HL
            reg->a = read8(cpu, reg->hl);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x7F:  //LD A, A
            reg->a = reg->a;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;
        
        // The next 16 instructions are all ADD and ADC

        case 0x80:  //ADD A, B
            set_N(0, cpu);
            set_H_add(reg->a, reg->b, cpu);
            set_C_add(reg->a, reg->b, cpu);
            reg->a += reg->b;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x81:  //ADD A, C
            set_N(0, cpu);
            set_H_add(reg->a, reg->c, cpu);
            set_C_add(reg->a, reg->c, cpu);
            reg->a += reg->c;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x82:  //ADD A, D
            set_N(0, cpu);
            set_H_add(reg->a, reg->d, cpu);
            set_C_add(reg->a, reg->d, cpu);
            reg->a += reg->d;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x83:  //ADD A, E
            set_N(0, cpu);
            set_H_add(reg->a, reg->e, cpu);
            set_C_add(reg->a, reg->e, cpu);
            reg->a += reg->e;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x84:  //ADD A, H
            set_N(0, cpu);
            set_H_add(reg->a, reg->h, cpu);
            set_C_add(reg->a, reg->h, cpu);
            reg->a += reg->h;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x85:  //ADD A, L
            set_N(0, cpu);
            set_H_add(reg->a, reg->l, cpu);
            set_C_add(reg->a, reg->l, cpu);
            reg->a += reg->l;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x86:  //ADD A, [HL]
            u8 = read8(cpu, reg->hl);
            set_N(0, cpu);
            set_H_add(reg->a, u8, cpu);
            set_C_add(reg->a, u8, cpu);
            reg->a += u8;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x87:  //ADD A, A
            set_N(0, cpu);
            set_H_add(reg->a, reg->a, cpu);
            set_C_add(reg->a, reg->a, cpu);
            reg->a += reg->a;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x88:  //ADC A, B
            u8 = reg->b + ((reg->f & FLAG_C)? 1 : 0);
            set_N(0, cpu);
            set_H_add(reg->a, u8, cpu);
            set_C_add(reg->a, u8, cpu);
            reg->a += u8;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x89:  //ADC A, C
            u8 = reg->c + ((reg->f & FLAG_C)? 1 : 0);
            set_N(0, cpu);
            set_H_add(reg->a, u8, cpu);
            set_C_add(reg->a, u8, cpu);
            reg->a += u8;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x8A:  //ADC A, D
            u8 = reg->d + ((reg->f & FLAG_C)? 1 : 0);
            set_N(0, cpu);
            set_H_add(reg->a, u8, cpu);
            set_C_add(reg->a, u8, cpu);
            reg->a += u8;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x8B:  //ADC A, E
            u8 = reg->e + ((reg->f & FLAG_C)? 1 : 0);
            set_N(0, cpu);
            set_H_add(reg->a, u8, cpu);
            set_C_add(reg->a, u8, cpu);
            reg->a += u8;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x8C:  //ADC A, H
            u8 = reg->h + ((reg->f & FLAG_C)? 1 : 0);
            set_N(0, cpu);
            set_H_add(reg->a, u8, cpu);
            set_C_add(reg->a, u8, cpu);
            reg->a += u8;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x8D:  //ADC A, L
            u8 = reg->l + ((reg->f & FLAG_C)? 1 : 0);
            set_N(0, cpu);
            set_H_add(reg->a, u8, cpu);
            set_C_add(reg->a, u8, cpu);
            reg->a += u8;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x8E:  //ADC A, [HL]
            u8 = read8(cpu, reg->hl);
            temp8 = ((reg->f & FLAG_C)? 1 : 0);
            u16 = reg->a + u8 + temp8;
            set_N(0, cpu);

            if (((reg->a & 0x0F) + (u8 & 0x0F) + temp8) > 0x0F) {
                set_H(1, cpu);
            } else {
                set_H(0, cpu);
            }
            
            if (u16 > 0xFF) {
                set_C(1, cpu);
            } else {
                set_C(0, cpu);
            }

            reg->a = (uint8_t)u16;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x8F:  //ADC A, A
            u8 = reg->a + ((reg->f & FLAG_C)? 1 : 0);
            set_N(0, cpu);
            set_H_add(reg->a, u8, cpu);
            set_C_add(reg->a, u8, cpu);
            reg->a += u8;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x90:  //SUB B
            set_N(1, cpu);
            set_H_sub(reg->a, reg->b, cpu);
            set_C_sub(reg->a, reg->b, cpu);
            reg->a -= reg->b;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x91:  //SUB C
            set_N(1, cpu);
            set_H_sub(reg->a, reg->c, cpu);
            set_C_sub(reg->a, reg->c, cpu);
            reg->a -= reg->c;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x92:  //SUB D
            set_N(1, cpu);
            set_H_sub(reg->a, reg->d, cpu);
            set_C_sub(reg->a, reg->d, cpu);
            reg->a -= reg->d;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x93:  //SUB E
            set_N(1, cpu);
            set_H_sub(reg->a, reg->e, cpu);
            set_C_sub(reg->a, reg->e, cpu);
            reg->a -= reg->e;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x94:  //SUB H
            set_N(1, cpu);
            set_H_sub(reg->a, reg->h, cpu);
            set_C_sub(reg->a, reg->h, cpu);
            reg->a -= reg->h;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x95:  //SUB L
            set_N(1, cpu);
            set_H_sub(reg->a, reg->l, cpu);
            set_C_sub(reg->a, reg->l, cpu);
            reg->a -= reg->l;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x96:  //SUB [HL]
            u8 = read8(cpu, reg->hl);
            set_N(1, cpu);
            set_H_sub(reg->a, u8, cpu);
            set_C_sub(reg->a, u8, cpu);
            reg->a -= u8;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x97:  //SUB A
            set_N(1, cpu);
            set_H_sub(reg->a, reg->a, cpu);
            set_C_sub(reg->a, reg->a, cpu);
            reg->a -= reg->a;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x98:  //SBC A, B
            u8 = reg->b + ((reg->f & FLAG_C) ? 1 : 0);
            set_N(1, cpu);
            set_H_sub(reg->a, u8, cpu);
            set_C_sbc(reg->a, u8, reg->f & FLAG_C, cpu);
            reg->a -= u8;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x99:  //SBC A, C
            u8 = reg->c + ((reg->f & FLAG_C) ? 1 : 0);
            set_N(1, cpu);
            set_H_sub(reg->a, u8, cpu);
            set_C_sbc(reg->a, u8, reg->f & FLAG_C, cpu);
            reg->a -= u8;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x9A:  //SBC A, D
            u8 = reg->d + ((reg->f & FLAG_C) ? 1 : 0);
            set_N(1, cpu);
            set_H_sub(reg->a, u8, cpu);
            set_C_sbc(reg->a, u8, reg->f & FLAG_C, cpu);
            reg->a -= u8;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x9B:  //SBC A, E
            u8 = reg->e + ((reg->f & FLAG_C) ? 1 : 0);
            set_N(1, cpu);
            set_H_sub(reg->a, u8, cpu);
            set_C_sbc(reg->a, u8, reg->f & FLAG_C, cpu);
            reg->a -= u8;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x9C:  //SBC A, H
            u8 = reg->h + ((reg->f & FLAG_C) ? 1 : 0);
            set_N(1, cpu);
            set_H_sub(reg->a, u8, cpu);
            set_C_sbc(reg->a, u8, reg->f & FLAG_C, cpu);
            reg->a -= u8;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0x9D:  //SBC A, L
            u8 = reg->l + ((reg->f & FLAG_C) ? 1 : 0);
            set_N(1, cpu);
            set_H_sub(reg->a, u8, cpu);
            set_C_sbc(reg->a, u8, reg->f & FLAG_C, cpu);
            reg->a -= u8;
            set_Z(reg->a, cpu);
            cpu->cycles += 1;
            cpu->pc += 1;
            break;

        case 0x9E:  //SBC A, HL
            u8 = read8(cpu, reg->hl);
            temp8 = ((reg->f & FLAG_C) ? 1 : 0);
            u16 = reg->a - u8 - temp8;
            set_N(1, cpu);

            if ((reg->a & 0x0F) < ((u8 & 0x0F) + temp8)) {
                set_H(1, cpu);
            } else {
                set_H(0, cpu);
            }
            
            if (u16 & 0xFF00) {
                set_C(1, cpu);
            } else {
                set_C(0, cpu);
            }

            reg->a = (uint8_t)u16;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0x9F:  //SBC A, A
            u8 = reg->a + ((reg->f & FLAG_C) ? 1 : 0);
            set_N(1, cpu);
            set_H_sub(reg->a, u8, cpu);
            set_C_sbc(reg->a, u8, reg->f & FLAG_C, cpu);
            reg->a -= u8;
            set_Z(reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xA0:  //AND B
            reg->a = (reg->a & reg->b);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xA1:  //AND C
            reg->a = (reg->a & reg->c);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xA2:  //AND D
            reg->a = (reg->a & reg->d);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xA3:  //AND E
            reg->a = (reg->a & reg->e);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xA4:  //AND H
            reg->a = (reg->a & reg->h);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xA5:  //AND L
            reg->a = (reg->a & reg->l);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xA6:  //AND [HL]
            reg->a = (reg->a & read8(cpu, reg->hl));
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0xA7:  //AND A
            reg->a = (reg->a & reg->a);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xA8:  //XOR B
            reg->a = (reg->a ^ reg->b);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xA9:  //XOR C
            reg->a = (reg->a ^ reg->c);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xAA:  //XOR D
            reg->a = (reg->a ^ reg->d);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xAB:  //XOR E
            reg->a = (reg->a ^ reg->e);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xAC:  //XOR H
            reg->a = (reg->a ^ reg->h);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xAD:  //XOR L
            reg->a = (reg->a ^ reg->l);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xAE:  //XOR HL
            reg->a = (reg->a ^ read8(cpu, reg->hl));
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0xAF:  //XOR A
            reg->a = (reg->a ^ reg->a);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xB0:  //OR B
            reg->a = (reg->a | reg->b);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xB1:  //OR C
            reg->a = (reg->a | reg->c);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xB2:  //OR D
            reg->a = (reg->a | reg->d);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xB3:  //OR E
            reg->a = (reg->a | reg->e);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xB4:  //OR H
            reg->a = (reg->a | reg->h);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xB5:  //OR L
            reg->a = (reg->a | reg->l);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

    case 0xB6:  //OR [HL]
            reg->a = (reg->a | read8(cpu, reg->hl));
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0xB7:  //OR A
            reg->a = (reg->a | reg->a);
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xB8:  //CP B
            temp8 = reg->a - reg->b;
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(reg->a, reg->b, cpu);
            set_C_sub(reg->a, reg->b, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xB9:  //CP C
            temp8 = reg->a - reg->c;
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(reg->a, reg->c, cpu);
            set_C_sub(reg->a, reg->c, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xBA:  //CP D
            temp8 = reg->a - reg->d;
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(reg->a, reg->d, cpu);
            set_C_sub(reg->a, reg->d, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xBB:  //CP E
            temp8 = reg->a - reg->e;
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(reg->a, reg->e, cpu);
            set_C_sub(reg->a, reg->e, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xBC:  //CP H
            temp8 = reg->a - reg->h;
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(reg->a, reg->h, cpu);
            set_C_sub(reg->a, reg->h, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xBD:  //CP L
            temp8 = reg->a - reg->l;
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(reg->a, reg->l, cpu);
            set_C_sub(reg->a, reg->l, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xBE:  //CP HL
            uint8_t val = read8(cpu, reg->hl);
            temp8 = reg->a - val;
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(reg->a, val, cpu);
            set_C_sub(reg->a, val, cpu);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0xBF:  //CP A
            temp8 = reg->a - reg->a;
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(reg->a, reg->a, cpu);
            set_C_sub(reg->a, reg->a, cpu);
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xC0:  //RET NZ
            if (!(reg->f & FLAG_Z)) {
                cpu->pc = stack_pop(cpu);
                cpu->cycles += 5;
            } 
            else {
                cpu->pc += 1;
                cpu->cycles += 2;
            }
            break;

        case 0xC1:  //POP BC
            reg->bc = stack_pop(cpu);
            cpu->pc += 1;
            cpu->cycles += 3;
            break;

        case 0xC2:  //JP NZ, u16
        // Jump to u16 if Z is not set
            if(!(reg->f & FLAG_Z)){
                cpu->pc = read16(cpu, cpu->pc+1);
                cpu->cycles += 4;
            }
            else{
                cpu->pc += 3;
                cpu->cycles += 3;
            }
            break;

        case 0xC3:  //JP u16
            // Jump to u16
            cpu->pc = read16(cpu, cpu->pc+1);
            //cpu->pc += 3;
            cpu->cycles += 4;
            break;

        case 0xC4:  //CALL NZ, u16
            u16 = read16(cpu, cpu->pc+1);
            cpu->pc += 3;
            if (!(reg->f & FLAG_Z)) {
                stack_push(cpu, cpu->pc);
                cpu->pc = u16;
                cpu->cycles += 5;
            } else {
                cpu->cycles += 3; 
            }
            break;

        case 0xC5:  //PUSH BC
            stack_push(cpu, reg->bc);
            cpu->pc += 1;
            cpu->cycles += 4;
            break;

        case 0xC6:  //ADD A, u8
            set_N(0, cpu);
            u8 = read8(cpu, cpu->pc+1);
            set_H_add(reg->a, u8, cpu);
            set_C_add(reg->a, u8, cpu);
            reg->a += u8;
            set_Z(reg->a, cpu);
            cpu->pc += 2;
            cpu->cycles += 2; 
            break;

        case 0xC7:  //RST 00H
            stack_push(cpu, cpu->pc+1);
            cpu->pc = 0x00;
            cpu->cycles += 4;
            break;

        case 0xC8:  //RET Z
            if (reg->f & FLAG_Z) {
                cpu->pc = stack_pop(cpu);
                cpu->cycles += 5;
            } 
            else {
                cpu->pc += 1;
                cpu->cycles += 2;
            }
            break;

        case 0xC9:  //RET
            cpu->pc = stack_pop(cpu);
            cpu->cycles += 4;
            break;

        case 0xCA:  //JP Z, u16
            if(reg->f & FLAG_Z){
                cpu->pc = read16(cpu, cpu->pc+1);
                cpu->cycles += 4;
            }
            else{
                cpu->pc += 3;
                cpu->cycles += 3;
            }
            break;

        case 0xCB:  //Prefix CB
            uint8_t opcode = read8(cpu, ++cpu->pc);
            run_pref_inst(cpu, opcode);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0xCC:  //CALL Z, u16
            u16 = read16(cpu, cpu->pc+1);
            cpu->pc += 3;
            if ((reg->f & FLAG_Z)) {
                stack_push(cpu, cpu->pc);
                cpu->pc = u16;
                cpu->cycles += 6;
            } else {
                cpu->cycles += 3; 
            }
            break;

        case 0xCD:  //CALL u16
            u16 = read16(cpu, cpu->pc+1);
            stack_push(cpu, cpu->pc+3);
            cpu->pc = u16;
            cpu->cycles += 6;
            break;

        case 0xCE:  //ADC A, u8
            u8 = read8(cpu, cpu->pc+1);
            temp8 = (reg->f & FLAG_C)? 1:0;
            u16 = reg->a + u8 + temp8;
            set_N(0, cpu);
            if (((reg->a & 0x0F) + (u8 & 0x0F) + temp8) > 0x0F)
                set_H(1, cpu); 
            else
                set_H(0, cpu);

            if (u16 > 0xFF) 
                set_C(1, cpu);
            else 
                set_C(0,cpu);
            reg->a = (uint8_t)u16;
            set_Z(reg->a, cpu);
            cpu->pc += 2;
            cpu->cycles += 2; 
            break;

        case 0xCF:  //RST 08H
            stack_push(cpu, cpu->pc+1);
            cpu->pc = 0x08;
            cpu->cycles += 4;
            break;

        case 0xD0:  //RET NC
            if (!(reg->f & FLAG_C)) {
                cpu->pc = stack_pop(cpu);
                cpu->cycles += 5;
            } else {
                cpu->pc += 1;
                cpu->cycles += 2;
            }
            break;

        case 0xD1:  //POP DE
            reg->de = stack_pop(cpu);
            cpu->pc += 1;
            cpu->cycles += 3;
            break;

        case 0xD2:  //JP NC, u16
            // Jump to u16 if C is  NOT set
            if(!(reg->f & FLAG_C)){
                cpu->pc = read16(cpu, cpu->pc+1);
                //cpu->pc += 3;
                cpu->cycles += 4;
            }
            else{
                cpu->pc += 3;
                cpu->cycles += 3; 
            }
            break;

        case 0xD3:  //HOLE
            cpu->pc += 1;
            break;

        case 0xD4:  //CALL NC, u16
            u16 = read16(cpu, cpu->pc+1);
            cpu->pc += 3;
            if (!(reg->f & FLAG_C)) {
                stack_push(cpu, cpu->pc);
                cpu->pc = u16;
                cpu->cycles += 5;
            } else {
                cpu->cycles += 3; 
            }
            break;

        case 0xD5:  //PUSH DE
            stack_push(cpu, reg->de);
            cpu->pc += 1;
            cpu->cycles += 4;
            break;

        case 0xD6:  //SUB A, u8
            // Subtract u8 from A
            set_N(1, cpu);
            u8 = read8(cpu, cpu->pc+1);
            set_H_sub(reg->a, u8, cpu);
            set_C_sub(reg->a, u8, cpu);
            reg->a -= u8;
            set_Z(reg->a, cpu);
            cpu->pc += 2;
            cpu->cycles += 2; 
            break;

        case 0xD7:  //RST 10H
            stack_push(cpu, cpu->pc+1);
            cpu->pc = 0x10;
            cpu->cycles += 4;
            break;

        case 0xD8:  //RET C
            if (reg->f & FLAG_C) {
                cpu->pc = stack_pop(cpu);
                cpu->cycles += 5;
            } else {
                cpu->pc += 1;
                cpu->cycles += 2;
            }
            break;

        case 0xD9:  //RETI
            // Set Interrupt Master Enable after RET
            cpu->pc = stack_pop(cpu);
            cpu->ime_enable = true;
            cpu->cycles += 4;
            break;

        case 0xDA:  //JP C, u16
            // Jump to u16 if C is set
            if(reg->f & FLAG_C){
                cpu->pc = read16(cpu, cpu->pc+1);
                cpu->cycles += 4;
            }
            else{
                cpu->pc += 3;
                cpu->cycles += 3;
            }
            break;

        case 0xDB:  //HOLE
            cpu->pc += 1;
            break;

        case 0xDC:  //CALL C, u16
            u16 = read16(cpu, cpu->pc+1);
            cpu->pc += 3;
            if (!(reg->f & FLAG_C)) {
                stack_push(cpu, cpu->pc);
                cpu->pc = u16;
                cpu->cycles += 5;
            } else {
                cpu->cycles += 3; 
            }
            break;

        case 0xDD:  //HOLE
            cpu->pc += 1;
            break;

        case 0xDE:  //SBC A, u8
            u8 = read8(cpu, cpu->pc+1); 
            temp8 = (reg->f & FLAG_C)? 1:0;
            u16 = reg->a - u8 - temp8;
            set_N(1, cpu);
            if((reg->a & 0x0F) < ((u8 & 0x0F) + temp8))
                set_H(1, cpu);  
            else
                set_H(0, cpu);
            
            if (u16 > 0xFF)
                set_C(1, cpu);
            else 
                set_C(0, cpu);

            reg->a = (uint8_t)u16;
            set_Z(reg->a, cpu);
            cpu->pc += 2;
            cpu->cycles += 2; 
            break;

        case 0xDF:  //RST 18H
            stack_push(cpu, cpu->pc+1);
            cpu->pc = 0x18;
            cpu->cycles += 4;
            break;

        case 0xE0:  //LDH u8, A
            u8 = read8(cpu, cpu->pc+1);
            write8(cpu, 0xFF00 + u8, reg->a);
            //printf("Got a write to: %04x\n\n\n\n\n\n\n\n", 0xFF00+u8);
            cpu->pc += 2;
            cpu->cycles += 3;
            break;

        case 0xE1:  //POP HL
            reg->hl = stack_pop(cpu);
            cpu->pc += 1;
            cpu->cycles += 3;
            break;

        case 0xE2:  //LD (C), A
            write8(cpu, 0xFF00 + reg->c, reg->a);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0xE3:  //HOLE
            cpu->pc += 1;
            break;

        case 0xE4:  //HOLE
            cpu->pc += 1;
            break;

        case 0xE5:  //PUSH HL
            stack_push(cpu, reg->hl);
            cpu->pc += 1;
            cpu->cycles += 4;
            break;

        case 0xE6:  //AND A, u8
            reg->a = (reg->a & read8(cpu, cpu->pc+1));
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(1, cpu);
            set_C(0, cpu);
            cpu->pc += 2;
            cpu->cycles += 2; 
            break;

        case 0xE7:  //RST 20H
            stack_push(cpu, cpu->pc+1);
            cpu->pc = 0x20;
            cpu->cycles += 4;
            break;

        case 0xE8:  //ADD SP, i8
            // 16bit ADD - signed i8 to sp
            set_Z(1, cpu);
            set_N(0, cpu);
            offset  = (int8_t)read8(cpu, cpu->pc+1);
            uint8_t low_sp = cpu->sp & 0xFF;
            uint8_t low_offset = offset;

            set_H(((cpu->sp & 0xF) + (offset & 0xF)) > 0xF, cpu);
            set_C(((low_sp & 0xFF) + (low_offset & 0xFF)) > 0xFF, cpu); // C seems to be using only the low bytes. 

            cpu->sp += offset;
            cpu->pc += 2;
            cpu->cycles += 4; 
            break;

        case 0xE9:  //JP HL
            // Jump to HL
            cpu->pc = reg->hl;
            cpu->cycles += 1; 
            break;

        case 0xEA:  //LD u16, A
            u16 = read16(cpu, cpu->pc+1);   // Read next two bytes as 16-bit address
            write8(cpu, u16, reg->a);  // Store A at that address
            cpu->pc += 3;
            cpu->cycles += 4;
            break;

        case 0xEB:  //HOLE
            cpu->pc += 1;
            break;

        case 0xEC:  //HOLE
            cpu->pc += 1;
            break;

        case 0xED:  //HOLE
            cpu->pc += 1;
            break;

        case 0xEE:  //XOR u8
            reg->a = (reg->a ^ read8(cpu, cpu->pc+1));
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 2;
            cpu->cycles += 2; 
            break;

        case 0xEF:  //RST 28H
            stack_push(cpu, cpu->pc+1);
            cpu->pc = 0x28;
            cpu->cycles += 4;
            break;

        case 0xF0:  //LDH A, u8
            u8 = read8(cpu, cpu->pc+1);
            reg->a = read8(cpu, 0xFF00 + u8);
            cpu->pc += 2;
            cpu->cycles += 3;
            break;

        case 0xF1:  //POP AF
            reg->af = stack_pop(cpu)  & 0xFFF0;
            cpu->pc += 1;
            cpu->cycles += 3;
            break;

        case 0xF2:  //LD A, C
            reg->a = read8(cpu, 0xFF00 + reg->c);
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0xF3:  //DI
            cpu->ime = false;
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xF4:  //HOLE
            cpu->pc += 1;
            break;

        case 0xF5:  //PUSH AF
            u16 = (reg->a << 8) | (reg->f & 0xF0);
            stack_push(cpu, u16);
            cpu->pc += 1;
            cpu->cycles += 4;
            break;

        case 0xF6:  //OR A, u8
            reg->a = (reg->a | read8(cpu, cpu->pc+1));
            set_Z(reg->a, cpu);
            set_N(0, cpu);
            set_H(0, cpu);
            set_C(0, cpu);
            cpu->pc += 2;
            cpu->cycles += 2; 
            break;

        case 0xF7:  //RST 30H
            stack_push(cpu, cpu->pc+1);
            cpu->pc = 0x30;
            cpu->cycles += 4;
            break;

        case 0xF8:  //LD HL, SP+i8
            offset = read8(cpu, cpu->pc+1);
            reg->hl = cpu->sp + offset;
            set_Z(1, cpu);
            set_N(0, cpu);

            if ((cpu->sp & 0xF) + (offset & 0xF) > 0xF)
                cpu->regs.f |= FLAG_H;
            else
                cpu->regs.f &= ~FLAG_H;

            if ((cpu->sp & 0xFF) + (offset & 0xFF) > 0xFF)
                cpu->regs.f |= FLAG_C;
            else
                cpu->regs.f &= ~FLAG_C;
            
            cpu->pc += 2;
            cpu->cycles += 3;
            break;

        case 0xF9:  //LD SP, HL
            cpu->sp = reg->hl;
            cpu->pc += 1;
            cpu->cycles += 2;
            break;

        case 0xFA:  //LD A, u16
            u16 = read16(cpu, cpu->pc+1);
            reg->a  = read8(cpu, u16);
            cpu->pc += 3;
            cpu->cycles += 4;
            break;

        case 0xFB:  //EI
            printf("Got an EI call. That means the test starts\n");
            cpu->ime_enable = true;  
            cpu->pc += 1;
            cpu->cycles += 1;
            break;

        case 0xFC:  //HOLE
            cpu->pc += 1;
            break;

        case 0xFD:  //HOLE
            cpu->pc += 1;
            break;

        case 0xFE:  //CP u8
            u8 = read8(cpu, cpu->pc+1);
            temp8 = reg->a - u8;
            set_Z(temp8, cpu);
            set_N(1, cpu);
            set_H_sub(reg->a, u8, cpu);
            set_C_sub(reg->a, u8, cpu);
            cpu->pc += 2;
            cpu->cycles += 2; 
            break;

        case 0xFF:  //RST 38H
            stack_push(cpu, cpu->pc+1);
            cpu->pc = 0x38;
            cpu->cycles += 4;
            break;
    }
}