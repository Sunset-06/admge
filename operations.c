#include "core.h";

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

}
*/

void run_inst(uint16_t opcode){
    switch(opcode){
        
    }
}