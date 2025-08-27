#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "cpu.h"

extern bool quit_flag;
extern bool bootrom_flag;
extern char serial_log[65536];  
extern size_t serial_len;

extern void serial_write(uint8_t value);
extern bool load_rom(CPU *cpu, const char* filename);

#endif  