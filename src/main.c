#include "emu.h"
#include "cpu.h"
#include "peripherals.h"

bool ime_enable = false;
bool quit_flag = false;
bool bootrom_flag = true;


char serial_log[65536];  
size_t serial_len = 0;
uint8_t *rom = NULL;
size_t rom_size = 0;



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

void dump_oam(CPU *cpu, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Failed to open %s for writing\n", filename);
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
        if (event.type == SDL_QUIT)
            quit_flag = true; 
        else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            //printf("Joypad state: %04x\n\n", cpu->joypad);
            bool is_pressed = (event.type == SDL_KEYDOWN);
            
            switch (event.key.keysym.sym) {
                // exit
                case SDLK_ESCAPE:
                    quit_flag = true;
                    break;
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
                case SDLK_RETURN:
                    is_pressed ? (cpu->joypad &= ~BUTTON_ST) : (cpu->joypad |= BUTTON_ST);
                    break;
                case SDLK_RSHIFT:
                    is_pressed ? (cpu->joypad &= ~BUTTON_SL) : (cpu->joypad |= BUTTON_SL);
                    break;
            }
        }
    }
    // request an interrupt if theres a change
    if (((last_joypad ^ cpu->joypad) & last_joypad) > 0)
        cpu->iflag |= (1 << 4); 

}

int main(int argc, char *argv[]) {
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
        start_cpu_noboot(&cpu); // This one does not need a bootrom
    
    if(!load_rom(&cpu, inputRom)){
        printf("Aborting...");
        return 1;
    }
    
    init_screen(4);
    init_audio(&cpu);   
    //FILE *full_dump = fopen("full_dump.txt", "w");

    while(!quit_flag){
        handle_input(&cpu);
        cpu_step(&cpu);
        //log_cpu_state(&cpu, full_dump);

        // Sleep if the audio buffer is too full lol
        int read_pos = atomic_load(&cpu.apu.read_pos);
        int write_pos = atomic_load(&cpu.apu.write_pos);
        int buffer_fullness = (write_pos - read_pos + AUDIO_BUFFER_SIZE) % AUDIO_BUFFER_SIZE;

        // more than 3/4th full = sleep() 
        if (buffer_fullness > (AUDIO_BUFFER_SIZE * 3 / 4)) {
             SDL_Delay(1); 
        }
    }
    //fclose(full_dump);
    destroy_audio();
    destroy_screen();
    free(rom);
}