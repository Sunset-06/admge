#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "cpu.h"

extern bool quit_flag;
extern bool ime_enable;
extern bool bootrom_flag;

extern void init_memory(CPU *cpu);
extern bool load_rom(CPU *cpu, const char* filename);

#endif  