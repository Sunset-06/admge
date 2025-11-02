#include "emu.h"
#include "cpu.h"
#include <stdio.h>
#include <string.h>

#define BOOT_ROM "./bootrom/bootix_dmg.bin"
//#define BOOT_ROM "./bootrom/dmg_boot.bin"

void serial_write(uint8_t value) {
    if (serial_len < sizeof(serial_log) - 1) {
        serial_log[serial_len++] = (char)value;
    }
}

bool load_rom(CPU *cpu, const char* filename) {
    FILE *bootromFile = fopen(BOOT_ROM, "rb");
    if (bootromFile == NULL) {
        printf("Error: Could not load boot ROM.\n");
        return false;
    }
    fread(cpu->bootrom, 1, 0x0100, bootromFile);
    fclose(bootromFile);
    
    FILE* romFile = fopen(filename, "rb");
    if (!romFile) {
        printf("Error reading ROM. Please check ROM file\n");
        return false;
    }

    fseek(romFile, 0, SEEK_END);
    rom_size = ftell(romFile);
    fseek(romFile, 0, SEEK_SET);

    rom = malloc(rom_size);
    if (rom == NULL) {
        printf("Error: Could not allocate memory for ROM\n");
        fclose(romFile);
        return false;
    }

    if (fread(rom, 1, rom_size, romFile) != rom_size) {
        printf("Error: Could not read the full ROM file\n");
        fclose(romFile);
        free(rom);
        return false;
    }
    fclose(romFile);
    
    memcpy(cpu->memory, rom, 0x8000);
    printf("Successfully loaded ROM. Size: %zu bytes\n", rom_size);
    cpu->mbc_type = rom[0x0147];
    printf("Cartridge Type: 0x%02X\n", cpu->mbc_type);
    return true;
}

uint8_t read8(CPU *cpu, uint16_t addr) {

    if (bootrom_flag && addr < 0x0100) {
        return cpu->bootrom[addr];
    }

    // read from rom
    if (addr <= 0x7FFF) {
        // Bank 00 is fixed
        if (addr <= 0x3FFF) {
            return rom[addr];
        }
        // 0x4000-0x7FFF --> switchable banks
        else {
            uint32_t offset = (cpu->curr_rom_bank * 0x4000) + (addr - 0x4000);
            return rom[offset];
        }
    }

    // cartridge ram
    if (addr >= 0xA000 && addr <= 0xBFFF) {
        if (cpu->ram_enabled) {
            uint32_t ram_offset = (cpu->curr_ram_bank * 0x2000) + (addr - 0xA000);
            return cpu->external_ram[ram_offset];
        }
        return 0xFF;
    }


    // VRAM access control
    if (addr >= 0x8000 && addr <= 0x9FFF) {
        // Only block if display is enabled AND in mode 3
        if ((cpu->ppu.lcdc & 0x80) && ((cpu->ppu.stat & 0x03) == 0x03)) {
            return 0xFF;
        }
        return cpu->memory[addr];
    }


    // Echo RAM
    if (addr >= 0xE000 && addr <= 0xFDFF) {
        return cpu->memory[addr - 0x2000];
    }

    // OAM access blocked during modes 2+3
    if ((addr >= 0xFE00 && addr <= 0xFE9F) && 
        ((cpu->ppu.stat & 0x03) >= 0x02)) {
        return 0xFF;
    }

    // Unusable memory
    if (addr >= 0xFEA0 && addr <= 0xFEFF) {
        return 0xFF;
    }

    // Inputs
    if (addr == 0xFF00){
        //0x20 - Select Buttons
        //0x10 - Select Dpad
        //0x8 - Start / D
        //0x4 - Select / U
        //0x2 - B / L
        //0x1 - A / R
        // Lower Nibble is Read-only
        //  0 is pressed and 1 is not
        // This extends to the two selection bits as well 
        uint8_t selection = cpu->memory[0xFF00];
        uint8_t result = 0xFF;

        result = (result & 0xCF) | (selection & 0x30);

        if ((selection & 0x10) == 0) {
            // Get D-pad state (Right, Left, Up, Down -> bits 0-3)
            uint8_t dpad_state = cpu->joypad & 0x0F;
            result &= (dpad_state | 0xF0); 
        }

        if ((selection & 0x20) == 0) {
            // Get button state (A,B,Select,Start -> bits 4-7)
            uint8_t button_state = (cpu->joypad >> 4) & 0x0F;
            result &= (button_state | 0xF0);
        }

        return result;
    }

    if (addr == 0xFF04) 
        return cpu->div;

    if (addr == 0xFF06) {
        return cpu->tma;
    }

    if (addr == 0xFF07) {
        return cpu->tac;
    }
    
    if (addr == 0xFF0F) {
        return cpu->iflag;
    }

    // Sound
    if (addr >= 0xFF10 && addr <= 0xFF3F) return apu_read(cpu, addr); 

    // VRAM
    if (addr >= 0xFF40 && addr <= 0xFF4B) return ppu_read(cpu, addr);
    
    if (addr == 0xFFFF) {
        return cpu->ie;
    }

    // PPU registers
    if (addr >= 0xFF40 && addr <= 0xFF4B) {
        return ppu_read(cpu, addr);
    }

    return cpu->memory[addr];
}

uint16_t read16(CPU *cpu, uint16_t addr) {
    return read8(cpu, addr) | (read8(cpu, addr + 1) << 8);
}

void write8(CPU *cpu, uint16_t addr, uint8_t value) {

    if (addr >= 0xA000 && addr <= 0xBFFF) {
        if (cpu->ram_enabled) {
            uint32_t ram_offset = (cpu->curr_ram_bank * 0x2000) + (addr - 0xA000);
            
            if (ram_offset < EX_RAM_SIZE) { 
                cpu->external_ram[ram_offset] = value;
            }
        }
        return;
    }
    
    // write to MBC
    if (addr <= 0x7FFF) {

        // ------------------- MBC1
        if (cpu->mbc_type >= 0x01 && cpu->mbc_type <= 0x03) {

            if (addr <= 0x1FFF) {
                cpu->ram_enabled = ((value & 0x0F) == 0x0A);
                return;
            }

            if (addr >= 0x2000 && addr <= 0x3FFF) {
                // The lower 5 bits of the value select the bank.
                uint8_t bank = value & 0x1F; 
                
                // bank 0 defaults to 1
                if (bank == 0) {
                    bank = 1;
                }
                
                cpu->curr_rom_bank = (cpu->curr_rom_bank & 0xE0) | bank;
                return;
            }

            if (addr >= 0x4000 && addr <= 0x5FFF) {
                if (cpu->bank_mode == 0) { 
                    cpu->curr_rom_bank = (cpu->curr_rom_bank & 0x1F) | ((value & 0x03) << 5);
                } else { 
                    cpu->curr_ram_bank = value & 0x03;
                }
                return;
            }
            
            if(addr >= 0x6000){
                cpu->bank_mode = value & 0x01;
                return;
            }
        }

        // ------------------- MBC2
        if (cpu->mbc_type == 0x05 || cpu->mbc_type == 0x06) {
            
            if (addr <= 0x3FFF) {
                // If A8 is CLEAR (0): RAM Enable
                if ((addr & 0x0100) == 0) {
                    // RAM is enabled if lower 4 bits of value is 0x0A
                    cpu->ram_enabled = ((value & 0x0F) == 0x0A);
                    return;
                } 
                
                else { 
                    uint8_t bank = value & 0x0F;
                    
                    if (bank == 0) {
                        bank = 1;
                    }

                    cpu->curr_rom_bank = bank; 
                    return;
                }
            }
        }
        return;

    }

    if (addr >= 0x8000 && addr <= 0x9FFF) {
        if ((cpu->ppu.lcdc & 0x80) && ((cpu->ppu.stat & 0x03) == 0x03)) {
            return; 
        }
    }

    // Echo RAM
    if (addr >= 0xE000 && addr <= 0xFDFF) {
        cpu->memory[addr - 0x2000] = value;
        return;
    }

    if (addr >= 0xFE00 && addr <= 0xFE9F) {
        //printf("Wrote to OAM region in vram\n");
        cpu->memory[addr] = value;
        return;
    }

    // Unusable memory
    if (addr >= 0xFEA0 && addr <= 0xFEFF) {
        return;
    }

    if (addr == 0xFF02) { // SC (Serial control)
        // If transfer start (bit 7 set, and using internal clock bit 0)
        if (value == 0x81) {
            uint8_t c = cpu->memory[0xFF01]; // read SB
            serial_write(c);

            cpu->memory[0xFF02] = 0; // clear SC
            return;
        }
    }

    // Write to DIV - resetting DIV
    if (addr == 0xFF04) {
        cpu->div = 0;
    }

    // Write to TMA
    if (addr == 0xFF06) {
        cpu->tma = value;
    }

    // Write to TAC
    if (addr == 0xFF07) {
        cpu->tac = value;
    }

    if (addr == 0xFF00) {
        cpu->memory[0xFF00] = (value & 0x30) | 0xCF; // lower nibble is read only here
    }

    // Interrupt Flag Register
    if (addr == 0xFF0F) {
        cpu->iflag = value;
        return;
    }

    if (addr >= 0xFF10 && addr <= 0xFF3F){
        apu_write(cpu, addr, value);
        return;
    }

    // PPU registers
    if (addr >= 0xFF40 && addr <= 0xFF4B) {
        ppu_write(cpu, addr, value);
        return;
    }

    // Boot ROM disable
    if (addr == 0xFF50 && cpu->bootrom_flag) {
        bootrom_flag = false;
        return;
    }

    // Interrupt Enable Register
    if (addr == 0xFFFF) {
        cpu->ie = value;
        return;
    }

    cpu->memory[addr] = value;
}

void write16(CPU *cpu, uint16_t addr, uint16_t value) {
    write8(cpu, addr, value & 0xFF);
    write8(cpu, addr + 1, (value >> 8) & 0xFF);
}

// ------------------------ STACK Functions ---------------------------//

void stack_push(CPU *cpu, uint16_t value) {
    cpu->sp -= 2;
    write16(cpu, cpu->sp, value);
}

uint16_t stack_pop(CPU *cpu) {
    uint16_t value = read16(cpu, cpu->sp);
    cpu->sp += 2;
    return value;
}