#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <stdio.h>

#define MEMORY_SIZE 0x10000  // 64KB

extern uint8_t memory[MEMORY_SIZE];  

void init_memory();
void load_rom(const char* filename);
uint8_t read_memory(uint16_t address);
void write_memory(uint16_t address, uint8_t value);

/*
Memory Map:

0000	3FFF	16 KiB ROM bank 00	    From cartridge, usually a fixed bank
4000	7FFF	16 KiB ROM Bank 01–NN	From cartridge, switchable bank via mapper (if any)
8000	9FFF	8 KiB Video RAM (VRAM)	In CGB mode, switchable bank 0/1
A000	BFFF	8 KiB                   External RAM	From cartridge, switchable bank if any
C000	CFFF	4 KiB Work RAM (WRAM)	
D000	DFFF	4 KiB Work RAM (WRAM)	In CGB mode, switchable bank 1–7
E000	FDFF	Echo RAM (mirror C000–DDFF)	
FE00	FE9F	Object attribute memory (OAM)	
FEA0	FEFF	Not Usable	            
FF00	FF7F	I/O Registers	
FF80	FFFE	High RAM (HRAM)	
FFFF	FFFF	Interrupt Enable register (IE)	
*/
#endif
