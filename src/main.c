#include "emu.h"
#include "cpu.h"
#include "ui.h"
#include "platform.h"

const uint64_t TIMEOUT_CYCLES = 20000000;
const int CYCLES_PER_FRAME = 70224;

float win_scale = 0.7;
bool ime_enable = false;
bool quit_flag = false;
bool bootrom_flag = true;
bool rom_loaded = false;
bool muted = false;
char* inputRom;
char serial_log[65536];  
size_t serial_len = 0;
uint8_t *rom = NULL;
size_t rom_size = 0;


// Palettes
const uint32_t* GAMEBOY_COLOURS = NULL;
const uint32_t MGB_COLOURS[4] = {
    0xFFFFFFFF, // White
    0xFFAAAAAA, // Light Gray
    0xFF555555, // Dark Gray
    0xFF000000  // Black
};
const uint32_t DMG_COLOURS[4] = {
    0xFF9BBC0F, // Lightest Green
    0xFF8BAC0F, // Light GreenT
    0xFF306230, // Dark Green
    0xFF0F380F  // Darkest Green
};

emu_mode current_mode = DMG;


int core_thread(void *ptr){

    CPU *cpu = (CPU *) ptr;
    uint64_t freq = SDL_GetPerformanceFrequency();
    uint64_t last_time = SDL_GetPerformanceCounter();
    double cycle_ctr = 0;
    uint64_t test_cycles = 0;

    while(!quit_flag){
        
        if(!rom_loaded){
            SDL_Delay(10);
            last_time = SDL_GetPerformanceCounter();
            continue;
        }

        uint64_t curr_time = SDL_GetPerformanceCounter();
        uint64_t ticks = curr_time - last_time;
        last_time = curr_time;
        cycle_ctr += ((double)ticks / (double)freq) * GB_CLOCK_SPEED;

        if(cycle_ctr > 70224 * 2)
            cycle_ctr = 70224 * 2;

        while(cycle_ctr > 0){
            if(current_mode == TEST){
                // headless test mode
                uint8_t opcode = read8(cpu, cpu->pc);
                // mooneye breakpoint
                if (opcode == 0x40) {
                    uint8_t b = (cpu->regs.bc >> 8) & 0xFF;
                    uint8_t c = cpu->regs.bc & 0xFF;
                    uint8_t d = (cpu->regs.de >> 8) & 0xFF;
                    uint8_t e = cpu->regs.de & 0xFF;
                    uint8_t h = (cpu->regs.hl >> 8) & 0xFF;
                    uint8_t l = cpu->regs.hl & 0xFF;

                    bool success = (b == 0x03 && c == 0x05 && d == 0x08 && 
                                    e == 0x0D && h == 0x15 && l == 0x22);

                    printf("\n--- Test Result ---\n");
                    if (success) {
                        printf("RESULT: PASSED\n");
                        exit(0); 
                    } else {
                        printf("RESULT: FAILED\n");
                        printf("Expected: 03 05 08 0D 15 22\n");
                        printf("Actual:   %02X %02X %02X %02X %02X %02X\n", b, c, d, e, h, l);
                        
                        // Print serial output
                        if (serial_len > 0) {
                            printf("Serial Output: %s\n", serial_log);
                        }
                        exit(1);
                    }
                }
                cpu_step(cpu);
                
                test_cycles++;
                if (test_cycles > TIMEOUT_CYCLES) {
                    printf("RESULT: TIMEOUT\n");
                    exit(1);
                }
            }
            else{ // ALL MODES THAT ARE NOT TEST
                cpu_step(cpu);
                //log_cpu_state(&cpu, full_dump);
                int read_pos = atomic_load(&cpu->apu.read_pos);
                int write_pos = atomic_load(&cpu->apu.write_pos);
                int buffer_fullness = (write_pos - read_pos + AUDIO_BUFFER_SIZE) % AUDIO_BUFFER_SIZE;

                while (buffer_fullness > (AUDIO_BUFFER_SIZE * 3 / 4)) {
                    SDL_Delay(1); 
                    read_pos = atomic_load(&cpu->apu.read_pos);
                    buffer_fullness = (write_pos - read_pos + AUDIO_BUFFER_SIZE) % AUDIO_BUFFER_SIZE;
                }
            }
            cycle_ctr -= cpu->cycles;
        }
    }
    return 0;

}

int main(int argc, char *argv[]) {
        
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-noboot") == 0) bootrom_flag = false;
        else if (strcmp(argv[i], "-debug") == 0) current_mode = DEBUG;
        else if (strcmp(argv[i], "-test")  == 0) current_mode = TEST;
        else if (strcmp(argv[i], "-mgb")   == 0) current_mode = MGB;
    }

    CPU cpu;
    //tex_scale = (current_mode == MGB)? 0.9 : 0.7;
    GAMEBOY_COLOURS = (current_mode == MGB)? MGB_COLOURS : DMG_COLOURS;

    if(bootrom_flag)
        start_cpu(&cpu); // This initializes everything normally - expects a bootrom
    else
        start_cpu_noboot(&cpu); // This one does not need a bootrom

    
    if (argc >= 2) {
        inputRom = argv[1];
        printf("%s\n", inputRom);
        if (load_rom(&cpu, inputRom)) {
            rom_loaded = true;
        }
    }
    
    
    if (current_mode != TEST){
        init_screen(4);
        init_audio(&cpu);
    }

    //FILE *full_dump = fopen("full_dump.txt", "w");
    SDL_Thread *emu_thread = SDL_CreateThread(core_thread, "admgeCore", &cpu);

    while (!quit_flag) {
        if (current_mode != TEST) {
            handle_input(&cpu);
            present_screen(&cpu.ppu, &cpu);
        }
        SDL_Delay(1);
    }
    SDL_WaitThread(emu_thread, NULL);

    //fclose(full_dump);
    
    if (current_mode != TEST) {
        destroy_audio();
        destroy_screen();
    }
    free(rom);
}
