#include "cpu.h"
#include <string.h>
#include <math.h>

static const int DUTY_TABLE[4][8] = {
    {0, 0, 0, 0, 0, 0, 0, 1}, // 12.5%
    {1, 0, 0, 0, 0, 0, 0, 1}, // 25%
    {1, 0, 0, 0, 0, 1, 1, 1}, // 50%
    {0, 1, 1, 1, 1, 1, 1, 0}  // 75%
};

static const int NOISE_TIMERS[8] = {8, 16, 32, 48, 64, 80, 96, 112};

void apu_init(APU *apu) {

    memset(apu, 0, sizeof(APU));
    atomic_init(&apu->write_pos, 0);
    atomic_init(&apu->read_pos, 0);
    apu->sample_counter = 0.0;
    apu->frame_seq_clock = 0;
    apu->frame_seq = 0;
}

static void ch1_trigger(APU *apu) {
    // If initial volume and envelope step are both 0, ch1 dac is off
    if ((apu->nr12 & 0xF8) == 0) {
        apu->ch1_enabled = false;
        apu->nr52 &= ~0x01; // Turn off status bit
        return;
    }
    apu->ch1_enabled = true;

    apu->nr52 |= 0x01;

    if (apu->nr14 & 0x40) { // Bit 6 = Length Enable
        apu->ch1_length_timer = 64 - (apu->nr11 & 0x3F);
    }

    uint16_t freq_data = ((apu->nr14 & 0x07) << 8) | apu->nr13;
    apu->ch1_timer = (2048 - freq_data) * 4;

    apu->ch1_envelope_volume = (apu->nr12 >> 4) & 0x0F;
    apu->ch1_envelope_timer = apu->nr12 & 0x07;
    
    // ch1 has a sweep
    uint8_t sweep_period = (apu->nr10 >> 4) & 0x07;
    uint8_t sweep_shift  = apu->nr10 & 0x07;

    apu->ch1_sweep_frequency = freq_data;

    apu->ch1_sweep_timer = (sweep_period == 0) ? 8 : sweep_period;

    apu->ch1_sweep_enabled = (sweep_period > 0) || (sweep_shift > 0);

    if (apu->ch1_sweep_enabled && sweep_shift > 0) {
        uint8_t sweep_dir = (apu->nr10 >> 3) & 0x01; // 0: add, 1: sub
        uint16_t new_freq = apu->ch1_sweep_frequency >> sweep_shift;
        
        if (sweep_dir == 1) { // Subtraction
            new_freq = apu->ch1_sweep_frequency - new_freq;
        } else { // Addition
            new_freq = apu->ch1_sweep_frequency + new_freq;
        }

        if (new_freq > 2047) {
            apu->ch1_enabled = false;
            apu->nr52 &= ~0x01;
        }
    }
}


static void ch2_trigger(APU *apu) {
    if ((apu->nr22 & 0xF8) == 0) {
        apu->ch2_enabled = false;
        apu->nr52 &= ~0x02; // Turn off status bit
        return;
    }
    apu->ch2_enabled = true;

    apu->nr52 |= 0x02; // Set status bit

    if (apu->nr24 & 0x40) {
        apu->ch2_length_timer = 64 - (apu->nr21 & 0x3F);
    }

    uint16_t freq_data = ((apu->nr24 & 0x07) << 8) | apu->nr23;
    apu->ch2_timer = (2048 - freq_data) * 4;

    apu->ch2_envelope_volume = (apu->nr22 >> 4) & 0x0F;
    apu->ch2_envelope_timer = apu->nr22 & 0x07;
}

static void ch3_trigger(APU *apu) {
    // check ch3 dac
    if ((apu->nr30 & 0x80) == 0) {
        apu->nr52 &= ~0x04;
        return;
    }

    apu->nr52 |= 0x04;

    // reload length timer
    apu->ch3_length_timer = 256 - apu->nr31;

    uint16_t freq_data = ((apu->nr34 & 0x07) << 8) | apu->nr33;
    apu->ch3_timer = (2048 - freq_data) * 2;
    
    apu->ch3_wave_pos = 0;
}

static void ch4_trigger(APU *apu) {
    if ((apu->nr42 & 0xF8) == 0) {
        apu->ch4_enabled = false;
        apu->nr52 &= ~0x08; // Turn off status bit
        return;
    }
    apu->ch4_enabled = true;

    apu->nr52 |= 0x08; // Set status bit

    if (apu->nr44 & 0x40) {
        apu->ch4_length_timer = 64 - (apu->nr41 & 0x3F);
    }

    apu->ch4_envelope_volume = (apu->nr42 >> 4) & 0x0F;
    apu->ch4_envelope_timer = apu->nr42 & 0x07;
    
    apu->ch4_lfsr = 0xFFFF; // Reset Linear Feedback Shift Register

    uint8_t clock_shift = apu->nr43 >> 4;
    uint8_t divisor_code = apu->nr43 & 0x07;
    apu->ch4_timer = NOISE_TIMERS[divisor_code] << clock_shift;
}

static void apu_step_timers(APU *apu, int t_cycles) {
    // Channel 1 (Pulse)
    if (apu->ch1_enabled) {
        apu->ch1_timer -= t_cycles;
        if (apu->ch1_timer <= 0) {
            uint16_t freq_data = ((apu->nr14 & 0x07) << 8) | apu->nr13;
            int period = (2048 - freq_data) * 4;
            apu->ch1_timer += period; // Reload timer

            apu->ch1_duty_pos = (apu->ch1_duty_pos + 1) % 8;
            
            // Get duty cycle output (0 or 1)
            uint8_t duty_pattern = (apu->nr11 >> 6) & 0x03;
            int duty_output = DUTY_TABLE[duty_pattern][apu->ch1_duty_pos];
            
            // Combine with envelope volume (0-15)
            uint8_t dac_input = duty_output * apu->ch1_envelope_volume;

            // Convert to float (-1.0 to 1.0)
            apu->ch1_dac_out = (dac_input / 7.5f) - 1.0f;
        }
    } else {
        apu->ch1_dac_out = 0.0f;
    }

    // Channel 2 (Pulse)
    if (apu->ch2_enabled) {
        apu->ch2_timer -= t_cycles;
        if (apu->ch2_timer <= 0) {
            uint16_t freq_data = ((apu->nr24 & 0x07) << 8) | apu->nr23;
            int period = (2048 - freq_data) * 4;
            apu->ch2_timer += period; // Reload timer
            
            apu->ch2_duty_pos = (apu->ch2_duty_pos + 1) % 8;

            uint8_t duty_pattern = (apu->nr21 >> 6) & 0x03;
            int duty_output = DUTY_TABLE[duty_pattern][apu->ch2_duty_pos];
            
            uint8_t dac_input = duty_output * apu->ch2_envelope_volume;
            apu->ch2_dac_out = (dac_input / 7.5f) - 1.0f;
        }
    } else {
        apu->ch2_dac_out = 0.0f;
    }

    // Channel 3 (Wave)
    // Check if DAC is on (bit 7 of NR30)
    if (apu->nr30 & 0x80) {
        apu->ch3_timer -= t_cycles;
        if (apu->ch3_timer <= 0) {
            uint16_t freq_data = ((apu->nr34 & 0x07) << 8) | apu->nr33;
            int period = (2048 - freq_data) * 2;
            apu->ch3_timer += period;
            
            // Advance wave position (0-31)
            apu->ch3_wave_pos = (apu->ch3_wave_pos + 1) % 32;
            
            // Get 4-bit sample from Wave RAM
            uint8_t sample_byte = apu->waveform[apu->ch3_wave_pos / 2];
            uint8_t sample_4bit = (apu->ch3_wave_pos % 2 == 0) 
                                ? (sample_byte >> 4)   // High nibble
                                : (sample_byte & 0x0F); // Low nibble
            
            // Apply volume shift (NR32)
            uint8_t vol_code = (apu->nr32 >> 5) & 0x03;
            if (vol_code == 1) sample_4bit >>= 0; // 100%
            else if (vol_code == 2) sample_4bit >>= 1; // 50%
            else if (vol_code == 3) sample_4bit >>= 2; // 25%
            else sample_4bit = 0; // 0%
            
            apu->ch3_dac_out = (sample_4bit / 7.5f) - 1.0f;
        }
    } else {
        apu->ch3_dac_out = 0.0f;
    }

    // Channel 4 (Noise)
    if (apu->ch4_enabled) {
        apu->ch4_timer -= t_cycles;
        if (apu->ch4_timer <= 0) {
            uint8_t clock_shift = apu->nr43 >> 4;
            uint8_t divisor_code = apu->nr43 & 0x07;
            
            // Reload timer
            apu->ch4_timer += NOISE_TIMERS[divisor_code] << clock_shift;

            // Clock the LFSR
            uint16_t lfsr = apu->ch4_lfsr;
            uint8_t xor_bit = (lfsr & 1) ^ ((lfsr >> 1) & 1);
            lfsr = (lfsr >> 1) | (xor_bit << 14);

            // Check for 7-bit "short mode"
            if (apu->nr43 & 0x08) {
                lfsr = (lfsr & 0xBF7F) | (xor_bit << 6);
            }
            apu->ch4_lfsr = lfsr;
            
            int duty_output = !(lfsr & 1); 
            
            uint8_t dac_input = duty_output * apu->ch4_envelope_volume;
            apu->ch4_dac_out = (dac_input / 7.5f) - 1.0f;
        }
    } else {
        apu->ch4_dac_out = 0.0f;
    }
}

static int16_t generate_mixed_sample(APU *apu) {
    // What I'm supposed to do
    // 1. Step all 4 channels (timers, envelopes, sweep)
    // 2. Get the current amplitude from each enabled channel
    // 3. Mix them together (simple addition)
    // 4. Return the final 16-bit sample.

    /* // the ultimate constant crakle 
    static double phase = 0.0;
    phase += 2.0 * 3.14159265358979323846 * FREQUENCY / SAMPLE_RATE;
    if (phase >= 2.0 * 3.14159265358979323846) {
        phase -= 2.0 * 3.14159265358979323846;
    }
    return (int16_t)(sin(phase) * AMPLITUDE); */

    // If master APU switch is off, return silence
    if ((apu->nr52 & 0x80) == 0) {
        return 0;
    }

    float left_mix = 0.0f;
    float right_mix = 0.0f;

    // (apu->nr52 & 0x0X) checks if channel is enabled
    if (apu->nr52 & 0x01) {
        if (apu->nr51 & 0x10) left_mix += apu->ch1_dac_out;
        if (apu->nr51 & 0x01) right_mix += apu->ch1_dac_out;
    }
    if (apu->nr52 & 0x02) {
        if (apu->nr51 & 0x20) left_mix += apu->ch2_dac_out;
        if (apu->nr51 & 0x02) right_mix += apu->ch2_dac_out;
    }
    if (apu->nr52 & 0x04) {
        if (apu->nr51 & 0x40) left_mix += apu->ch3_dac_out;
        if (apu->nr51 & 0x04) right_mix += apu->ch3_dac_out;
    }
    if (apu->nr52 & 0x08) {
        if (apu->nr51 & 0x80) left_mix += apu->ch4_dac_out;
        if (apu->nr51 & 0x08) right_mix += apu->ch4_dac_out;
    }

    float left_vol = ((apu->nr50 >> 4) & 0x07) / 7.0f;
    float right_vol = (apu->nr50 & 0x07) / 7.0f;

    float final_left = left_mix * left_vol;
    float final_right = right_mix * right_vol;
    
    float mono_mix = (final_left + final_right) / 2.0f;

    int16_t sample = (int16_t)(mono_mix * 8000.0f); 

    return sample;
}

// In apu.c, add these new functions:

static void apu_step_length(APU *apu) {
    // Channel 1
    if ((apu->nr14 & 0x40) && apu->ch1_length_timer > 0) { // Length enabled?
        apu->ch1_length_timer--;
        if (apu->ch1_length_timer == 0) {
            apu->ch1_enabled = false;
            apu->nr52 &= ~0x01; // Update status bit
        }
    }
    
    // Channel 2
    if ((apu->nr24 & 0x40) && apu->ch2_length_timer > 0) {
        apu->ch2_length_timer--;
        if (apu->ch2_length_timer == 0) {
            apu->ch2_enabled = false;
            apu->nr52 &= ~0x02;
        }
    }

    // Channel 3
    if ((apu->nr34 & 0x40) && apu->ch3_length_timer > 0) {
        apu->ch3_length_timer--;
        if (apu->ch3_length_timer == 0) {
            apu->nr52 &= ~0x04; // Just clear status bit
        }
    }

    // Channel 4
    if ((apu->nr44 & 0x40) && apu->ch4_length_timer > 0) {
        apu->ch4_length_timer--;
        if (apu->ch4_length_timer == 0) {
            apu->ch4_enabled = false;
            apu->nr52 &= ~0x08;
        }
    }
}

static void apu_step_envelope(APU *apu) {
    // Channel 1
    uint8_t ch1_period = apu->nr12 & 0x07;
    if (ch1_period > 0) {
        if (apu->ch1_envelope_timer > 0) {
            apu->ch1_envelope_timer--;
        }
        if (apu->ch1_envelope_timer == 0) {
            apu->ch1_envelope_timer = ch1_period; // Reload
            
            uint8_t direction = (apu->nr12 & 0x08);
            uint8_t current_vol = apu->ch1_envelope_volume;
            
            if (direction && current_vol < 15) {
                apu->ch1_envelope_volume++;
            } else if (!direction && current_vol > 0) {
                apu->ch1_envelope_volume--;
            }
        }
    }

    // Channel 2
    uint8_t ch2_period = apu->nr22 & 0x07;
    if (ch2_period > 0) {
        if (apu->ch2_envelope_timer > 0) {
            apu->ch2_envelope_timer--;
        }
        if (apu->ch2_envelope_timer == 0) {
            apu->ch2_envelope_timer = ch2_period;
            uint8_t direction = (apu->nr22 & 0x08);
            uint8_t current_vol = apu->ch2_envelope_volume;
            if (direction && current_vol < 15) {
                apu->ch2_envelope_volume++;
            } else if (!direction && current_vol > 0) {
                apu->ch2_envelope_volume--;
            }
        }
    }

    // Channel 4
    uint8_t ch4_period = apu->nr42 & 0x07;
    if (ch4_period > 0) {
        if (apu->ch4_envelope_timer > 0) {
            apu->ch4_envelope_timer--;
        }
        if (apu->ch4_envelope_timer == 0) {
            apu->ch4_envelope_timer = ch4_period;
            uint8_t direction = (apu->nr42 & 0x08);
            uint8_t current_vol = apu->ch4_envelope_volume;
            if (direction && current_vol < 15) {
                apu->ch4_envelope_volume++;
            } else if (!direction && current_vol > 0) {
                apu->ch4_envelope_volume--;
            }
        }
    }
}

static void apu_step_sweep(APU *apu) {
    if (!apu->ch1_sweep_enabled) {
        return;
    }

    if (apu->ch1_sweep_timer > 0) {
        apu->ch1_sweep_timer--;
    }
    
    if (apu->ch1_sweep_timer == 0) {
        uint8_t sweep_period = (apu->nr10 >> 4) & 0x07;
        apu->ch1_sweep_timer = (sweep_period == 0) ? 8 : sweep_period; // Reload

        if (sweep_period > 0) {
            // Run the sweep calculation
            uint8_t sweep_shift = apu->nr10 & 0x07;
            uint16_t new_freq = apu->ch1_sweep_frequency >> sweep_shift;
            
            if (apu->nr10 & 0x08) { // Subtraction
                new_freq = apu->ch1_sweep_frequency - new_freq;
            } else { // Addition
                new_freq = apu->ch1_sweep_frequency + new_freq;
            }

            // Check for overflow and update
            if (new_freq > 2047) {
                apu->ch1_enabled = false;
                apu->nr52 &= ~0x01;
            } else if (sweep_shift > 0) {
                apu->ch1_sweep_frequency = new_freq;
                
                // Update the actual channel 1 frequency registers
                apu->nr13 = new_freq & 0xFF;
                apu->nr14 = (apu->nr14 & 0xF8) | ((new_freq >> 8) & 0x07);
                
                // Run the overflow check again (hardware quirk)
                uint16_t check_freq = new_freq >> sweep_shift;
                if (apu->nr10 & 0x08) check_freq = new_freq - check_freq;
                else check_freq = new_freq + check_freq;
                
                if (check_freq > 2047) {
                     apu->ch1_enabled = false;
                     apu->nr52 &= ~0x01;
                }
            }
        }
    }
}

static void apu_step_frame_sequencer(APU *apu, uint8_t step) {
    // The sequencer clocks different components on different steps:
    // Step 0: Length
    // Step 2: Length, Sweep
    // Step 4: Length
    // Step 6: Length, Sweep
    // Step 7: Envelope

    if (step % 2 == 0) { // Steps 0, 2, 4, 6
        apu_step_length(apu);
    }
    if (step == 2 || step == 6) { // Steps 2, 6
        apu_step_sweep(apu);
    }
    if (step == 7) { // Step 7
        apu_step_envelope(apu);
    }
}


/**
 * This is the "Producer" function.
 * It's called by the main emulator loop (cpu_step).
 * It's job is to generate samples based on CPU cycles
 * and push them into the ring buffer.
 */
void apu_step(APU *apu, CPU *cpu) {

    int t_cycles = cpu->cycles * 4;

    apu->sample_counter += t_cycles;

    apu->frame_seq_clock += t_cycles;
    if (apu->frame_seq_clock >= 8192) {
        apu->frame_seq_clock -= 8192;
        apu_step_frame_sequencer(apu, apu->frame_seq);
        apu->frame_seq = (apu->frame_seq + 1) % 8;
    }

    if ((apu->nr52 & 0x80)) { // Only step timers if APU is on
        apu_step_timers(apu, t_cycles);
    }

    // Only generate a new cycle if enough cycles have passed
    while (apu->sample_counter >= CYCLES_PER_SAMPLE) {
        apu->sample_counter -= CYCLES_PER_SAMPLE;

        int16_t mono_sample = generate_mixed_sample(apu);

        int write_pos = atomic_load(&apu->write_pos);
        int read_pos = atomic_load(&apu->read_pos);

        int next_write_pos = (write_pos + 2) % AUDIO_BUFFER_SIZE;

        if (next_write_pos == read_pos) {
            // buffer got too full :(
            // fatass
            break; 
        }

        // Apparently we just duplicate the mono sample for stereo
        apu->internal_buffer[write_pos] = mono_sample; // Left Channel
        apu->internal_buffer[(write_pos + 1) % AUDIO_BUFFER_SIZE] = mono_sample; // Right Channel
        
        atomic_store(&apu->write_pos, next_write_pos);
    }
}

uint8_t apu_read(CPU *cpu, uint16_t addr) {
    APU *apu = &cpu->apu;

    // NR52 (0xFF26) is always readable
    if (addr == 0xFF26) {
        return (apu->nr52 & 0x8F) | 0x70;
    }

    // Wave RAM (0xFF30 - 0xFF3F)
    if (addr >= 0xFF30 && addr <= 0xFF3F) {
        if ((apu->nr52 & 0x80) == 0 || (apu->nr52 & 0x04) == 0) {
            return apu->waveform[addr - 0xFF30];
        }
        // If apu and ch3 are on, access is blocked
        return 0xFF;
    }

    // apu is off
    if ((apu->nr52 & 0x80) == 0) {
        return 0xFF;
    }

    switch (addr) {
        // --- Channel 1 ---
        case 0xFF10: return apu->nr10 | 0x80;
        case 0xFF11: return apu->nr11 | 0x3F; 
        case 0xFF12: return apu->nr12;
        case 0xFF13: return 0xFF; // Write-only
        case 0xFF14: return apu->nr14 | 0xBF;

        // --- Channel 2 ---
        case 0xFF15: return 0xFF; // Unused
        case 0xFF16: return apu->nr21 | 0x3F;
        case 0xFF17: return apu->nr22;
        case 0xFF18: return 0xFF; // Write-only
        case 0xFF19: return apu->nr24 | 0xBF;

        // --- Channel 3 ---
        case 0xFF1A: return apu->nr30 | 0x7F;
        case 0xFF1B: return 0xFF; // Write-only
        case 0xFF1C: return apu->nr32 | 0x9F;
        case 0xFF1D: return 0xFF; // Write-only
        case 0xFF1E: return apu->nr34 | 0xBF;

        // --- Channel 4 ---
        case 0xFF1F: return 0xFF; // Unused
        case 0xFF20: return 0xFF; // Write-only
        case 0xFF21: return apu->nr42;
        case 0xFF22: return apu->nr43;
        case 0xFF23: return apu->nr44 | 0xBF;

        case 0xFF24: return apu->nr50;
        case 0xFF25: return apu->nr51;
    }
    return 0xFF;
}

void apu_write(CPU *cpu, uint16_t addr, uint8_t value) {

    APU *apu = &cpu->apu;
    if (addr == 0xFF26) {
        apu->nr52 = (value & 0x80) | (apu->nr52 & 0x0F);
        
        if ((apu->nr52 & 0x80) == 0) {
            // Clear everything.
            memset(&apu->nr10, 0, 0xFF26 - 0xFF10);
        }
        return;
    }

    // If APU is off, writes are ignored.
    if ((apu->nr52 & 0x80) == 0) {
        return;
    }

    // Handle Waveform
    if (addr >= 0xFF30 && addr <= 0xFF3F) {
        // Only writable if ch3 is off
        if ((apu->nr52 & 0x80) == 0 || (apu->nr52 & 0x04) == 0) {
            apu->waveform[addr - 0xFF30] = value;
        }
        return;
    }

    // nr register writes
    switch (addr) {
        // Channel 1
        case 0xFF10: apu->nr10 = value; break;
        case 0xFF11: apu->nr11 = value; break;
        case 0xFF12: apu->nr12 = value; break;
        case 0xFF13: apu->nr13 = value; break;
        case 0xFF14: 
            apu->nr14 = value; 
            if (value & 0x80) {
                ch1_trigger(apu);
            }
            break;

        // Channel 2
        case 0xFF16: apu->nr21 = value; break;
        case 0xFF17: apu->nr22 = value; break;
        case 0xFF18: apu->nr23 = value; break;
        case 0xFF19: 
            apu->nr24 = value; 
            if (value & 0x80) {
                ch2_trigger(apu);
            }
            break;

        // Channel 3
        case 0xFF1A: apu->nr30 = value; break;
        case 0xFF1B: apu->nr31 = value; break;
        case 0xFF1C: apu->nr32 = value; break;
        case 0xFF1D: apu->nr33 = value; break;
        case 0xFF1E: 
            apu->nr34 = value; 
            if (value & 0x80) {
                ch3_trigger(apu);
            }
            break;

        // Channel 4
        case 0xFF20: apu->nr41 = value; break;
        case 0xFF21: apu->nr42 = value; break;
        case 0xFF22: apu->nr43 = value; break;
        case 0xFF23: 
            apu->nr44 = value; 
            if (value & 0x80) {
                ch4_trigger(apu);
            }
            break;

        // Control
        case 0xFF24: apu->nr50 = value; break;
        case 0xFF25: apu->nr51 = value; break;
    }
}