#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "cpu.h"

extern bool quit_flag;
extern bool bootrom_flag;
extern int timer_counter;
extern char serial_log[65536];  
extern size_t serial_len;

extern void serial_write(uint8_t value);
extern void init_memory(CPU *cpu);
extern bool load_rom(CPU *cpu, const char* filename);

#endif  