#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "cpu.h"

extern bool quit_flag;
extern bool ime_enable;

extern void init_memory(CPU *cpu);
extern void load_rom(CPU *cpu, const char* filename);
extern void run_inst(uint16_t opcode, CPU *cpu);
extern void run_pref_inst(CPU *cpu);

#endif  