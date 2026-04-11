#include "emu.h"
#include "cpu.h"
#include "ui.h"
#include "platform.h"

float tex_scale = 0.7;
bool ime_enable = false;
bool quit_flag = false;
bool bootrom_flag = true;

char* inputRom;
char serial_log[65536];  
size_t serial_len = 0;
uint8_t *rom = NULL;
size_t rom_size = 0;

// Palettes
const uint32_t* GAMEBOY_COLOURS = NULL;
const uint32_t SGB_COLOURS[4] = {
    0xFFFFFFFF, // White
    0xFFAAAAAA, // Light Gray
    0xFF555555, // Dark Gray
    0xFF000000  // Black
};
const uint32_t DMG_COLOURS[4] = {
    0xFF9BBC0F, // Lightest Green
    0xFF8BAC0F, // Light Green
    0xFF306230, // Dark Green
    0xFF0F380F  // Darkest Green
};

emu_mode current_mode = DMG;


void main_loop(CPU *cpu){
    while(!quit_flag){
        if(current_mode == TEST){
            // test mode
        }
        else{
            handle_input(cpu);
            cpu_step(cpu);
            //log_cpu_state(&cpu, full_dump);
            
            // Sleep if the audio buffer is too full lol
            int read_pos = atomic_load(&cpu->apu.read_pos);
            int write_pos = atomic_load(&cpu->apu.write_pos);
            int buffer_fullness = (write_pos - read_pos + AUDIO_BUFFER_SIZE) % AUDIO_BUFFER_SIZE;

            // more than 3/4th full = sleep() 
            if (buffer_fullness > (AUDIO_BUFFER_SIZE * 3 / 4))
                SDL_Delay(1);
        }
    }
}


int main(int argc, char *argv[]) {
    if(argc < 2){
        printf("Wrong start, use it like this:\n admge path/to/your/rom -noboot(optional)\n Aborting...");
        return 1;
    }
        
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-noboot") == 0) bootrom_flag = false;
        else if (strcmp(argv[i], "-debug") == 0) current_mode = DEBUG;
        else if (strcmp(argv[i], "-test")  == 0) current_mode = TEST;
        else if (strcmp(argv[i], "-sgb")   == 0) current_mode = SGB;
    }

    CPU cpu;
    inputRom = argv[1];
    tex_scale = (current_mode == SGB)? 0.6 : 0.7;
    GAMEBOY_COLOURS = (current_mode == SGB)? SGB_COLOURS : DMG_COLOURS;

    if(bootrom_flag)
        start_cpu(&cpu); // This initializes everything normally - expects a bootrom
    else
        start_cpu_noboot(&cpu); // This one does not need a bootrom
    
    if(!load_rom(&cpu, inputRom)){
        printf("Aborting...");
        return 1;
    }


    if (current_mode != TEST){
        init_screen(4);
        init_audio(&cpu);
    }

    //FILE *full_dump = fopen("full_dump.txt", "w");

    main_loop(&cpu);

    //fclose(full_dump);
    
    if (current_mode != TEST) {
        destroy_audio();
        destroy_screen();
    }
    free(rom);
}