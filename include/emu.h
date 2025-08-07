#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

extern uint8_t quit_flag;
extern bool ime_enable;

void init_memory(CPU *cpu);
void load_rom(CPU *cpu, const char* filename);


#endif  