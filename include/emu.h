#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

struct CPU;
typedef struct CPU CPU;

typedef enum {
  DMG,
  MGB,
  DEBUG,
  TEST
} emu_mode;

extern emu_mode current_mode;

extern const uint32_t MGB_COLOURS[4];
extern const uint32_t DMG_COLOURS[4];

extern bool quit_flag;
extern bool bootrom_flag;
extern char serial_log[65536];
extern char* inputRom;
extern size_t serial_len;
extern uint8_t *rom;
extern size_t rom_size;

extern void serial_write(uint8_t value);
extern bool load_rom(CPU *cpu, const char* filename);
extern void name_sav(const char* romFile, char* saveFile) ;
extern void load_sav(CPU *cpu, const char* romFile);
extern void save_sav(CPU *cpu, const char* romFile);
extern void handle_input(CPU* cpu);

#endif  
