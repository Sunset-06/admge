#include "emu.h"
#include "cpu.h"
#include "screen.h"

bool ime_enable = false;
bool quit_flag = false;
bool bootrom_flag = true;


char serial_log[65536];  
size_t serial_len = 0;

void dump_serial_log(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return;

    fwrite(serial_log, 1, serial_len, f);
    fclose(f);
}


void dump_vram(CPU *cpu, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Failed to open %s for writing\n", filename);
        return;
    }

    // Dump entire VRAM (0x8000–0x9FFF)
    fwrite(&cpu->memory[0x8000], 1, 0x2000, file);
    fclose(file);
    printf("VRAM dumped to %s\n", filename);
}

void dump_header(CPU *cpu, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Failed to open %s for writing\n", filename);
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

int main(int argc, char *argv[]) {
    /* Intializating everything */
    if(argc < 2){
        printf("Wrong start, use it like this:\n admge path/to/your/rom -noboot(optional)\n Aborting...");
        return 1;
    }

    if(argc >= 3 && strcmp(argv[2], "-noboot") == 0)
        bootrom_flag = false;

    CPU cpu;
    char* inputRom = argv[1];
    if(bootrom_flag)
        start_cpu(&cpu); // This initializes everything normally - expects a bootrom
    else
        start_cpu_noboot(&cpu);
    
    if(!load_rom(&cpu, inputRom)){
        printf("Aborting...");
        return 1;
    }
    
    init_screen(4);
    FILE *full_dump = fopen("full_dump.txt", "w");

    while(!quit_flag){
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit_flag = true; 
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit_flag = true;  
                }
                if (event.key.keysym.sym == SDLK_SPACE) {
                    dump_vram(&cpu, "vram.bin");
                    dump_header(&cpu, "header.bin");
                    dump_serial_log("serial.txt");
                }
            }
        }

        cpu_step(&cpu);
        //log_cpu_state(&cpu, full_dump);
    }
    fclose(full_dump);
    sdl_destroy();
}