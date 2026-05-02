#include "emu.h"
#include "cpu.h"
#include "ui.h"

void dump_serial_log(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return;  
    fwrite(serial_log, 1, serial_len, f);
    fclose(f);
}


void dump_vram(CPU *cpu, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Failed to open %s for writing\n", filename);
        return;
    }

    // Dump entire VRAM (0x8000–0x9FFF)
    fwrite(&cpu->memory[0x8000], 1, 0x2000, file);
    fclose(file);
    printf("VRAM dumped to %s\n", filename);
}

void dump_oam(CPU *cpu, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Failed to open %s for writing\n", filename);
        return;
    }

    // Dump entire OAM (0xFE00–0xFE9F)
    fwrite(&cpu->memory[0xFE00], 1, 0x9F, file);
    fclose(file);
    printf("OAM dumped to %s\n", filename);
}

void dump_header(CPU *cpu, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Failed to open %s for writing\n", filename);
        return;
    }

    fwrite(&cpu->memory[0x0100], 1, 0x50, file);
    fclose(file);
    printf("Header dumped to %s\n", filename);
}

/* Logs the entire CPU state to passed file*/
void log_cpu_state(const CPU *cpu, FILE *log_file) {

    // Extract individual flags from the F register
    // Z (Zero Flag) is bit 7
    // N (Subtract Flag) is bit 6
    // H (Half Carry Flag) is bit 5
    // C (Carry Flag) is bit 4
    uint8_t z_flag = (cpu->regs.f & 0x80) ? 1 : 0;
    uint8_t n_flag = (cpu->regs.f & 0x40) ? 1 : 0;
    uint8_t h_flag = (cpu->regs.f & 0x20) ? 1 : 0;
    uint8_t c_flag = (cpu->regs.f & 0x10) ? 1 : 0;

    // Read the opcode and the next two bytes for context
    uint8_t opcode = cpu->memory[cpu->pc];
    uint8_t operand1 = cpu->memory[cpu->pc + 1];
    uint8_t operand2 = cpu->memory[cpu->pc + 2];

    // Print the formatted state to the file
    fprintf(log_file,
            "A: %02X F: %02X B: %02X C: %02X D: %02X E: %02X H: %02X L: %02X "
            "SP:%04X PC:%04X | "
            "Flags(ZNHC):%d%d%d%d | "
            "IME:%d | "
            "PC: %02X %02X %02X | "
            "Cycles: %lu \n",
            cpu->regs.a, cpu->regs.f, cpu->regs.b, cpu->regs.c, cpu->regs.d, cpu->regs.e, cpu->regs.h, cpu->regs.l,
            cpu->sp, cpu->pc,
            z_flag, n_flag, h_flag, c_flag,
            cpu->ime,
            opcode, operand1, operand2,
            cpu->cycles);
}

// Useful for masking the cpu->joypad variable - updates the cpu input state
const uint8_t BUTTON_R = 1; // Bit 0
const uint8_t BUTTON_L = 1 << 1; // Bit 1
const uint8_t BUTTON_U = 1 << 2; // Bit 2
const uint8_t BUTTON_D = 1 << 3; // Bit 3
const uint8_t BUTTON_A = 1 << 4; // Bit 4
const uint8_t BUTTON_B = 1 << 5; // Bit 5
const uint8_t BUTTON_SL = 1 << 6; // Bit 6
const uint8_t BUTTON_ST = 1 << 7; // Bit 7

void handle_input(CPU* cpu) {
    SDL_Event event;
    uint8_t last_joypad = cpu->joypad;
    while (SDL_PollEvent(&event)) {
        ui_handle_event(&event);
        if (event.type == SDL_QUIT)
            SDL_AtomicSet(&quit_flag, 1);

        if((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && event.key.keysym.sym == SDLK_q)
            SDL_AtomicSet(&quit_flag, 1);

        if (!ui_want_capture()) {
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                //printf("Joypad state: %04x\n\n", cpu->joypad);
                bool is_pressed = (event.type == SDL_KEYDOWN);
                
                switch (event.key.keysym.sym) {
                    // for logging
                    case SDLK_SPACE:
                        dump_vram(cpu, "vram.bin");
                        dump_oam(cpu, "oam.bin");
                        dump_header(cpu, "header.bin");
                        dump_serial_log("serial.txt");
                        break;
                    // DPad
                    case SDLK_RIGHT:
                        is_pressed ? (cpu->joypad &= ~BUTTON_R) : (cpu->joypad |= BUTTON_R);
                        break;
                    case SDLK_LEFT:
                        is_pressed ? (cpu->joypad &= ~BUTTON_L) : (cpu->joypad |= BUTTON_L);
                        break;
                    case SDLK_UP:
                        is_pressed ? (cpu->joypad &= ~BUTTON_U) : (cpu->joypad |= BUTTON_U);
                        break;
                    case SDLK_DOWN:
                        is_pressed ? (cpu->joypad &= ~BUTTON_D) : (cpu->joypad |= BUTTON_D);
                        break;
                    // Buttons (A, B, Sl, St)
                    case SDLK_z:
                        is_pressed ? (cpu->joypad &= ~BUTTON_A) : (cpu->joypad |= BUTTON_A);
                        break;
                    case SDLK_x:
                        is_pressed ? (cpu->joypad &= ~BUTTON_B) : (cpu->joypad |= BUTTON_B);
                        break;
                    case SDLK_m:
                        if(is_pressed) SDL_AtomicSet(&muted, !SDL_AtomicGet(&muted));
                        break;
                    case SDLK_RETURN:
                        is_pressed ? (cpu->joypad &= ~BUTTON_ST) : (cpu->joypad |= BUTTON_ST);
                        break;
                    case SDLK_RSHIFT: // v
                    case SDLK_LSHIFT:
                        is_pressed ? (cpu->joypad &= ~BUTTON_SL) : (cpu->joypad |= BUTTON_SL);
                        break;
                }
            }
        }
    }
    // request an interrupt if theres a change
    if (((last_joypad ^ cpu->joypad) & last_joypad) > 0)
        cpu->iflag |= (1 << 4); 

}

void name_sav(const char* romFile, char* saveFile) {
    strcpy(saveFile, romFile);
    char* dot = strrchr(saveFile, '.');
    if (dot) {
        strcpy(dot, ".sav");
    } else {
        strcat(saveFile, ".sav");
    }
}

void save_sav(CPU *cpu, const char* romFile) {
    char save_path[256];
    name_sav(romFile, save_path);

    FILE* f = fopen(save_path, "wb");
    if (!f) {
        printf("Error: Failed to open save: %s\n", save_path);
        return;
    }
    //mbc2
    if (cpu->mbc_type == 0x05 || cpu->mbc_type == 0x06) {
        fwrite(cpu->mbc2_ram, 1, sizeof(cpu->mbc2_ram), f);
    } 
    // everything else
    else
        fwrite(cpu->external_ram, 1, EX_RAM_SIZE, f);

    fclose(f);
    printf("Saved at %s\n", save_path);
}

void load_sav(CPU *cpu, const char* romFile) {
    char save_path[256];
    name_sav(romFile, save_path);

    FILE* f = fopen(save_path, "rb");
    if (!f) {
        return;
    }

    if (cpu->mbc_type == 0x05 || cpu->mbc_type == 0x06) {
        fread(cpu->mbc2_ram, 1, sizeof(cpu->mbc2_ram), f);
    } else {
        fread(cpu->external_ram, 1, EX_RAM_SIZE, f);
    }

    fclose(f);
    // printf("Save loaded from %s\n", save_path);
}
