#include "cpu.h"
#include <string.h>
#include <math.h>

void apu_init(APU *apu) {

    memset(apu, 0, sizeof(APU));
    atomic_init(&apu->write_pos, 0);
    atomic_init(&apu->read_pos, 0);
    apu->sample_counter = 0.0;
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
}

static int16_t generate_mixed_sample(APU *apu) {
    // What I'm supposed to do
    // 1. Step all 4 channels (timers, envelopes, sweep)
    // 2. Get the current amplitude from each enabled channel
    // 3. Mix them together (simple addition)
    // 4. Return the final 16-bit sample.

    // the ultimate constant crakle 
    static double phase = 0.0;
    phase += 2.0 * 3.14159265358979323846 * FREQUENCY / SAMPLE_RATE;
    if (phase >= 2.0 * 3.14159265358979323846) {
        phase -= 2.0 * 3.14159265358979323846;
    }
    return (int16_t)(sin(phase) * AMPLITUDE);
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

    // apu->frame_seq_clock += t_cycles;

    // Check if enough cycles have passed to generate a new sample
    while (apu->sample_counter >= CYCLES_PER_SAMPLE) {
        apu->sample_counter -= CYCLES_PER_SAMPLE;

        int16_t mono_sample = generate_mixed_sample(apu);

        // current positions
        int write_pos = atomic_load(&apu->write_pos);
        int read_pos = atomic_load(&apu->read_pos);

        // We are writing 2 samples (L+R), so check 2 spots ahead
        int next_write_pos = (write_pos + 2) % AUDIO_BUFFER_SIZE;

        if (next_write_pos == read_pos) {
            // buffer got too full :(
            // fatass
            break; 
        }

        // Apparently we just duplicate the mono sample for stereo, which  is kinda funny
        apu->internal_buffer[write_pos] = mono_sample; // Left Channel
        apu->internal_buffer[(write_pos + 1) % AUDIO_BUFFER_SIZE] = mono_sample; // Right Channel
        
        atomic_store(&apu->write_pos, next_write_pos);
    }
}

uint8_t apu_read(CPU *cpu, uint16_t addr) {
    // Why are you even reading this commit, look for something newer
    return 0xFF;
}

void apu_write(CPU *cpu, uint16_t addr, uint8_t value) {

    APU *apu = &cpu->apu;
    // Writing to NR52 (0xFF26) bit 7 controls master power, all registers (0xFF10-0xFF25) are cleared.
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
        // This RAM is only accessible when Channel 3 is OFF.
        apu->waveform[addr - 0xFF30] = value;
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