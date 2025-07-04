#include "cpu.h";
#include "mem.h";
/*
    Use Registers like this:
    Registers cpu = { 0 };

    cpu.a = 0x12;
    cpu.f = 0x34;
    cpu.b = 0x56;
    cpu.c = 0x78;
    cpu.h = 0x9A;
    cpu.l = 0xBC;
    cpu.sp = 0xFFFE;
    cpu.pc = 0x0100;

    printf("A: 0x%02X\n", cpu.a);
    printf("F: 0x%02X\n", cpu.f);
    printf("AF: 0x%04X\n", cpu.af);
    printf("BC: 0x%04X\n", cpu.bc);
    printf("HL: 0x%04X\n", cpu.hl);
    printf("SP: 0x%04X\n", cpu.sp);
    printf("PC: 0x%04X\n", cpu.pc);
}*/
void run_inst(uint16_t opcode, Registers *cpu){
    
    switch(opcode){
        case 0x00:  //NOP
            break;

        case 0x01:  //LD BC, u16 - 3bytes
            // write u16 into BC
            uint16_t u16 = memory[++pc];
            u16 = u16 | (memory[++pc] << 8);
            cpu->bc = u16;
            break;

        case 0x02:  //LD BC, A 
            //write from A to BC
            cpu->bc = (cpu->bc & 0xFF00) | cpu->a;    
            break;

        case 0x03:  //INC BC
            //Increment value of BC by 1
            cpu->bc += 1;
            break;

        case 0x04:  //INC B
            //Increment value of B by 1
            cpu->b += 1;
            //Change the Z flag
            if (cpu->b == 0) {
                cpu->f |= FLAG_Z;
            } else {
                cpu->f &= ~FLAG_Z;
            }
            // Clear N flag
            cpu->f &= ~FLAG_N;
            // Change the H  flag
            if ((cpu->b & 0x0F) == 0x00) {
                cpu->f |= FLAG_H;
            } else {
                cpu->f &= ~FLAG_H;
            }
            break;

        case 0x05:  //DEC B
            // Decrease value of B by 1
            cpu->b -= 1;
            // Change Z flag
            if (cpu->b == 0) {
                cpu->f |= FLAG_Z;
            } else {
                cpu->f &= ~FLAG_Z;
            }
            break;

        case 0x06:  //LD B, u8
            // Write u8 into B
            cpu->b = memory[++pc];
            break;
            
        case 0x07:  //RLCA
            // Rotate Left A
            break;

        case 0x08:  //LD u16, SP     
            // Copy SP & $FF at address u16 and SP >> 8 at address u16 + 1.
            break;

        case 0x09:  //ADD HL, BC
            // Add value of BC  into HL
            break;

        case 0x0A:  //LD A, BC
            // Write Byte pointed by BC into A
            break;
        
        case 0x0B:  //DEC BC
            // Decrement value in BC by 1
            break;  

        case 0x0C:  //INC C
            // Increment value in C by 1
            break;  

        case 0x0D:  //DEC C
            // Decrement value in C by 1
            break;  

        case 0x0E:  //LD C, u8
            // Write u8 into C
            break;  

        case 0x0F:  //RRCA
            // Rotate Register A right
            break;
            
        case 0x10:  //STOP
            // STOP - Check ths one before implementing
            break;

        case 0x11:  //LD DE, u16
            // Load u16 into DE
            break;

        case 0x12:  //LD DE, A
            // Copy the value in register A into the byte pointed to by r16.
            break;

        case 0x13:  //INC DE
            // Increment value in DE by 1
            break;

        case 0x14:  //INC D
            // Incremenrt value in D by 1
            break;

        case 0x15:  //DEC D
            // Decrease value in D by 1
            break;
        
        case 0x16:  //LD D, u8
            // Load u8 into D
            break;
        
        case 0x17:  //RLA
            // Roatate A left through carry
            break;

        case 0x18:  //JR i8
            // Jump relative to i8
            break;
        
        case 0x19:  //ADD HL, DE
            // Add value of DE to HL
            break;

        case 0x1A:  //LD A, DE
            // Decrease value in D by 1
            break;
        
        case 0x1B:  //DEC DE
            break;

        case 0x1C:  //INC E
            break;

        case 0x1D:  //DEC E
            break;

        case 0x1E:  //LD E, u8
            break;

        case 0x1F:  //RRA
            break;

        case 0x20:  //JR NZ, i8
            break;

        case 0x21:  //LD HL, u16
            break;

        case 0x22:  //LD HL+, A
            break;

        case 0x23:  //INC HL
            break;

        case 0x24:  //INC H
            break;

        case 0x25:  //DEC H
            break;

        case 0x26:  //LD H, u8
            break;

        case 0x27:  //DAA
            break;

        case 0x28:  //JR Z, i8
            break;

        case 0x29:  //ADD HL, HL
            break;

        case 0x2A:  //LD A, HL+
            break;

        case 0x2B:  //DEC HL
            break;

        case 0x2C:  //INC L
            break;

        case 0x2D:  //DEC L
            break;

        case 0x2E:  //LD L, u8
            break;

        case 0x2F:  //CPL
            break;

        case 0x30:  //JR NC, i8
            break;

        case 0x31:  //LD SP, u16
            break;

        case 0x32:  //LD HL-, A
            break;

        case 0x33:  //INC SP
            break;

        case 0x34:  //INC HL
            break;

        case 0x35:  //DEC HL
            break;

        case 0x36:  //LD HL, u8
            break;

        case 0x37:  //SCF
            break;

        case 0x38:  //JR C, i8
            break;

        case 0x39:  //ADD HL, SP
            break;

        case 0x3A:  //LD A, HL-
            break;

        case 0x3B:  //DEC SP
            break;

        case 0x3C:  //INC A
            break;

        case 0x3D:  //DEC A
            break;

        case 0x3E:  //LD A, u8
            break;

        case 0x3F:  //CCF
            break;

        case 0x40:  //LD B, B
            break;

        case 0x41:  //LD B, C
            break;

        case 0x42:  //LD B, D
            break;

        case 0x43:  //LD B, E
            break;

        case 0x44:  //LD B, H
            break;

        case 0x45:  //LD B, L
            break;

        case 0x46:  //LD B, HL
            break;

        case 0x47:  //LD B, A
            break;

        case 0x48:  //LD C, B
            break;

        case 0x49:  //LD C, C
            break;

        case 0x4A:  //LD C, D
            break;

        case 0x4B:  //LD C, E
            break;

        case 0x4C:  //LD C, H
            break;

        case 0x4D:  //LD C, L
            break;

        case 0x4E:  //LD C, HL
            break;

        case 0x4F:  //LD C, A
            break;

        case 0x50:  //LD D, B
            break;

        case 0x51:  //LD D, C
            break;

        case 0x52:  //LD D, D
            break;

        case 0x53:  //LD D, E
            break;

        case 0x54:  //LD D, H
            break;

        case 0x55:  //LD D, L
            break;

        case 0x56:  //LD D, HL
            break;

        case 0x57:  //LD D, A
            break;

        case 0x58:  //LD E, B
            break;

        case 0x59:  //LD E, C
            break;

        case 0x5A:  //LD E, D
            break;

        case 0x5B:  //LD E, E
            break;

        case 0x5C:  //LD E, H
            break;

        case 0x5D:  //LD E, L
            break;

        case 0x5E:  //LD E, HL
            break;

        case 0x5F:  //LD E, A
            break;

        case 0x60:  //LD H, B
            break;

        case 0x61:  //LD H, C
            break;

        case 0x62:  //LD H, D
            break;

        case 0x63:  //LD H, E
            break;

        case 0x64:  //LD H, H
            break;

        case 0x65:  //LD H, L
            break;

        case 0x66:  //LD H, HL
            break;

        case 0x67:  //LD H, A
            break;

        case 0x68:  //LD L, B
            break;

        case 0x69:  //LD L, C
            break;

        case 0x6A:  //LD L, D
            break;

        case 0x6B:  //LD L, E
            break;

        case 0x6C:  //LD L, H
            break;

        case 0x6D:  //LD L, L
            break;

        case 0x6E:  //LD L, HL
            break;

        case 0x6F:  //LD L, A
            break;

        case 0x70:  //LD HL, B
            break;

        case 0x71:  //LD HL, C
            break;

        case 0x72:  //LD HL, D
            break;

        case 0x73:  //LD HL, E
            break;

        case 0x74:  //LD HL, H
            break;

        case 0x75:  //LD HL, L
            break;

        case 0x76:  //HALT
            break;

        case 0x77:  //LD HL, A
            break;

        case 0x78:  //LD A, B
            break;

        case 0x79:  //LD A, C
            break;

        case 0x7A:  //LD A, D
            break;

        case 0x7B:  //LD A, E
            break;

        case 0x7C:  //LD A, H
            break;

        case 0x7D:  //LD A, L
            break;

        case 0x7E:  //LD A, HL
            break;

        case 0x7F:  //LD A, A
            break;

        case 0x80:  //ADD A, B
            break;

        case 0x81:  //ADD A, C
            break;

        case 0x82:  //ADD A, D
            break;

        case 0x83:  //ADD A, E
            break;

        case 0x84:  //ADD A, H
            break;

        case 0x85:  //ADD A, L
            break;

        case 0x86:  //ADD A, HL
            break;

        case 0x87:  //ADD A, A
            break;

        case 0x88:  //ADC A, B
            break;

        case 0x89:  //ADC A, C
            break;

        case 0x8A:  //ADC A, D
            break;

        case 0x8B:  //ADC A, E
            break;

        case 0x8C:  //ADC A, H
            break;

        case 0x8D:  //ADC A, L
            break;

        case 0x8E:  //ADC A, HL
            break;

        case 0x8F:  //ADC A, A
            break;

        case 0x90:  //SUB B
            break;

        case 0x91:  //SUB C
            break;

        case 0x92:  //SUB D
            break;

        case 0x93:  //SUB E
            break;

        case 0x94:  //SUB H
            break;

        case 0x95:  //SUB L
            break;

        case 0x96:  //SUB HL
            break;

        case 0x97:  //SUB A
            break;

        case 0x98:  //SBC A, B
            break;

        case 0x99:  //SBC A, C
            break;

        case 0x9A:  //SBC A, D
            break;

        case 0x9B:  //SBC A, E
            break;

        case 0x9C:  //SBC A, H
            break;

        case 0x9D:  //SBC A, L
            break;

        case 0x9E:  //SBC A, HL
            break;

        case 0x9F:  //SBC A, A
            break;

        case 0xA0:  //AND B
            break;

        case 0xA1:  //AND C
            break;

        case 0xA2:  //AND D
            break;

        case 0xA3:  //AND E
            break;

        case 0xA4:  //AND H
            break;

        case 0xA5:  //AND L
            break;

        case 0xA6:  //AND HL
            break;

        case 0xA7:  //AND A
            break;

        case 0xA8:  //XOR B
            break;

        case 0xA9:  //XOR C
            break;

        case 0xAA:  //XOR D
            break;

        case 0xAB:  //XOR E
            break;

        case 0xAC:  //XOR H
            break;

        case 0xAD:  //XOR L
            break;

        case 0xAE:  //XOR HL
            break;

        case 0xAF:  //XOR A
            break;

        case 0xB0:  //OR B
            break;

        case 0xB1:  //OR C
            break;

        case 0xB2:  //OR D
            break;

        case 0xB3:  //OR E
            break;

        case 0xB4:  //OR H
            break;

        case 0xB5:  //OR L
            break;

        case 0xB6:  //OR HL
            break;

        case 0xB7:  //OR A
            break;

        case 0xB8:  //CP B
            break;

        case 0xB9:  //CP C
            break;

        case 0xBA:  //CP D
            break;

        case 0xBB:  //CP E
            break;

        case 0xBC:  //CP H
            break;

        case 0xBD:  //CP L
            break;

        case 0xBE:  //CP HL
            break;

        case 0xBF:  //CP A
            break;

        case 0xC0:  //RET NZ
            break;

        case 0xC1:  //POP BC
            break;

        case 0xC2:  //JP NZ, u16
            break;

        case 0xC3:  //JP u16
            break;

        case 0xC4:  //CALL NZ, u16
            break;

        case 0xC5:  //PUSH BC
            break;

        case 0xC6:  //ADD A, u8
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
            break;

        case 0xCC:  //CALL Z, u16
            break;

        case 0xCD:  //CALL u16
            break;

        case 0xCE:  //ADC A, u8
            break;

        case 0xCF:  //RST 08H
            break;

        case 0xD0:  //RET NC
            break;

        case 0xD1:  //POP DE
            break;

        case 0xD2:  //JP NC, u16
            break;

        case 0xD3:  //UNUSED
            break;

        case 0xD4:  //CALL NC, u16
            break;

        case 0xD5:  //PUSH DE
            break;

        case 0xD6:  //SUB u8
            break;

        case 0xD7:  //RST 10H
            break;

        case 0xD8:  //RET C
            break;

        case 0xD9:  //RETI
            break;

        case 0xDA:  //JP C, u16
            break;

        case 0xDB:  //UNUSED
            break;

        case 0xDC:  //CALL C, u16
            break;

        case 0xDD:  //UNUSED
            break;

        case 0xDE:  //SBC A, u8
            break;

        case 0xDF:  //RST 18H
            break;

        case 0xE0:  //LDH u8, A
            break;

        case 0xE1:  //POP HL
            break;

        case 0xE2:  //LD C, A
            break;

        case 0xE3:  //UNUSED
            break;

        case 0xE4:  //UNUSED
            break;

        case 0xE5:  //PUSH HL
            break;

        case 0xE6:  //AND u8
            break;

        case 0xE7:  //RST 20H
            break;

        case 0xE8:  //ADD SP, i8
            break;

        case 0xE9:  //JP HL
            break;

        case 0xEA:  //LD u16, A
            break;

        case 0xEB:  //UNUSED
            break;

        case 0xEC:  //UNUSED
            break;

        case 0xED:  //UNUSED
            break;

        case 0xEE:  //XOR u8
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

        case 0xF4:  //UNUSED
            break;

        case 0xF5:  //PUSH AF
            break;

        case 0xF6:  //OR u8
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

        case 0xFC:  //UNUSED
            break;

        case 0xFD:  //UNUSED
            break;

        case 0xFE:  //CP u8
            break;

        case 0xFF:  //RST 38H
            break;
    }
}