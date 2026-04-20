#include "emu.h"
#include "cpu.h"
#include <stdio.h>
#include <string.h>

//#define BOOT_ROM "./bootrom/bootix_dmg.bin"
#define BOOT_ROM "./bootrom/dmg_boot.bin"

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
    load_sav(cpu, filename);
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
          if ((cpu->mbc_type >= 0x01 && cpu->mbc_type <= 0x03) && cpu->bank_mode == 1) {
            uint32_t bank_offset = (cpu->curr_ram_bank << 5) * 0x4000;
            return rom[(bank_offset + addr) % rom_size];
          }
          return rom[addr];
        }
        // 0x4000-0x7FFF --> switchable banks
        else {
            uint32_t offset = (cpu->curr_rom_bank * 0x4000) + (addr - 0x4000);
            if (offset < rom_size) {
                return rom[offset];
            }
            return 0xFF;
        }
    }

    // cartridge ram
    if (addr >= 0xA000 && addr <= 0xBFFF) {
        if (cpu->ram_enabled) {
            // MBC2
            if (cpu->mbc_type == 0x05 || cpu->mbc_type == 0x06) {
                // MBC2 ram is mirrored.
                uint16_t offset = addr & 0x01FF;
                return cpu->mbc2_ram[offset] | 0xF0; 
            }
            // All other MBCs
            else{
                uint32_t ram_offset = (cpu->curr_ram_bank * 0x2000) + (addr - 0xA000);
                if (ram_offset < EX_RAM_SIZE) {
                    return cpu->external_ram[ram_offset];
                }
                // RTC registers
                if (cpu->rtc.sel >= 0x08 && cpu->rtc.sel <= 0x0C) {
                    return cpu->rtc.latch[cpu->rtc.sel - 0x08];
                }
                // RAM banking
                if (ram_offset < EX_RAM_SIZE) {
                    return cpu->external_ram[ram_offset];
                }
            }
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

    if (addr >= 0xFF00 && addr <= 0xFF7F){
        switch (addr){
            //Inputs
            case 0xFF00:

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

            // Serial Control
            case 0xFF02:
                return cpu->memory[0xFF02] | 0x7E;
            
            case 0xFF04: 
                return cpu->div;

            case 0xFF05: 
                return cpu->tima;

            case 0xFF06:
                return cpu->tma;
            
            case 0xFF07:
                return cpu->tac | 0xF8;
            
            case 0xFF0F:
                return cpu->iflag | 0xE0;
            
            // Sound
            case 0xFF10 ... 0xFF3F:
                return apu_read(cpu, addr);
            
            // VRAM
            case 0xFF40:
                return ppu_read(cpu, addr);

            case 0xFF41: // STAT - Bit 7 is always 1
                return ppu_read(cpu, addr) | 0x80;
            
            case 0xFF42 ... 0xFF4B:
                return ppu_read(cpu, addr);
            
            default: return 0xFF;
        }
    }
    
    if (addr == 0xFFFF) {
        return cpu->ie;
    }

    return cpu->memory[addr];
}

uint16_t read16(CPU *cpu, uint16_t addr) {
    return read8(cpu, addr) | (read8(cpu, addr + 1) << 8);
}

void write8(CPU *cpu, uint16_t addr, uint8_t value) {
    
    // write to MBC
    if (addr <= 0x7FFF) {

        // ------------------- MBC1
        if (cpu->mbc_type >= 0x01 && cpu->mbc_type <= 0x03) {

            if (addr <= 0x1FFF) {
                bool was_enabled = cpu->ram_enabled;
                cpu->ram_enabled = ((value & 0x0F) == 0x0A);
                if (was_enabled && !cpu->ram_enabled) {
                    save_sav(cpu, inputRom);
                }
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
                uint8_t bits = value & 0x03;
                cpu->curr_ram_bank = bits;
                cpu->curr_rom_bank = (cpu->curr_rom_bank & 0x1F) | (bits << 5);
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
                    
                    if (bank == 0)
                        bank = 1;

                    cpu->curr_rom_bank = bank; 
                    return;
                }
            }
        }

        // ------------------- MBC3 
        if (cpu->mbc_type >= 0x0F && cpu->mbc_type <= 0x13) {
            // RAM Enable
            if (addr <= 0x1FFF) {
                bool was_enabled = cpu->ram_enabled;
                cpu->ram_enabled = ((value & 0x0F) == 0x0A);

                if (was_enabled && !cpu->ram_enabled) {
                    save_sav(cpu, inputRom);
                }
            }
            else if (addr >= 0x2000 && addr <= 0x3FFF) {
                uint8_t bank = value & 0x7F;
                if (bank == 0) bank = 1;
                cpu->curr_rom_bank = bank;
            }
            // ram bank or RTC reg Select
            else if (addr >= 0x4000 && addr <= 0x5FFF) {
                if (value <= 0x07) {
                    cpu->curr_ram_bank = value;
                    cpu->rtc.sel = 0; // Unselect RTC
                }
                else if (value >= 0x08 && value <= 0x0C)
                    cpu->rtc.sel = value;
            }
            // Latch Clock
            else if (addr >= 0x6000 && addr <= 0x7FFF) {
                if (cpu->rtc.latch_val == 0x00 && value == 0x01) {
                    update_rtc(cpu);
                    for (int i = 0; i < 5; i++) {
                        cpu->rtc.latch[i] = cpu->rtc.main[i]; //copying values to the latch (snapshot of rtc)
                    }
                }
                cpu->rtc.latch_val = value;
            }
            return;
        }
        return;

    }

    if (addr >= 0x8000 && addr <= 0x9FFF) {
        if ((cpu->ppu.lcdc & 0x80) && ((cpu->ppu.stat & 0x03) == 0x03)) {
            return; 
        }
    }

    if (addr >= 0xA000 && addr <= 0xBFFF) {
        if (cpu->ram_enabled) {
            // MBC2
            if (cpu->mbc_type == 0x05 || cpu->mbc_type == 0x06) {
                // MBC2 mirrored ram
                uint16_t offset = (addr - 0xA000) % 0x200;
                cpu->mbc2_ram[offset] = value & 0x0F;
            }
            // Other MBCs
            else{
                uint32_t ram_offset = (cpu->curr_ram_bank * 0x2000) + (addr - 0xA000);
                
                if (ram_offset < EX_RAM_SIZE) { 
                    cpu->external_ram[ram_offset] = value;
                }
            }
        }
        return;
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
        if (value & 0x80) {
            uint8_t c = cpu->memory[0xFF01]; // read SB
            serial_write(c);

            cpu->memory[0xFF02] = value & 0x7F; // clear SC
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
