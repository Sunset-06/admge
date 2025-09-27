#include "cpu.h"
#include "audio.h"

const int CYCLES_PER_SAMPLE = CPU_FREQUENCY / SAMPLE_RATE;
/*

Cartridge writes to specific audio registers as to what it wants CH!,2 and 4 to do. 
This involves using  nrx1,2,3 and 4.
This data will then be added to my audiobuffer based on a fixed frequency, and the second sdl thread
will also read the buffer at the same frequency. 


This means that my APU needs to do these every step:

Advancing the internal state of the four APU channels based on how many cycles have passed.

Checking the Frame Sequencer to see if length, envelope, or sweep timers need to be clocked.

Generating audio samples at the correct rate (4,194,304 CPU cycles / 44,100 samples per second ≈ 95 cycles per sample).

Mixing the output of the four channels together.

Placing these generated samples into a separate, intermediate buffer*/

/* There will be a separate thread that consumes this buffer. This thread will run synced to real time NOT the cpu cycles. 
   I'll be placing this in my SDL_audio.c*/

void apu_init(APU *apu) {
    // 1. Zero out all internal state and most registers.
    // This correctly initializes all internal timers, counters, enabled flags,
    // and volume levels to an "off" or "zero" state.
    memset(apu, 0, sizeof(APU));

    // 2. Set the specific register values left by the boot ROM.
    // These values are what a game cartridge sees upon startup.
    apu->nr10 = 0x80;
    apu->nr11 = 0xBF;
    apu->nr12 = 0xF3;
    apu->nr14 = 0xBF;
    apu->nr21 = 0x3F;
    apu->nr24 = 0xBF;
    apu->nr30 = 0x7F;
    apu->nr32 = 0x9F;
    apu->nr34 = 0xBF;
    apu->nr41 = 0xFF;
    apu->nr44 = 0xBF;
    apu->nr50 = 0x77; 
    apu->nr51 = 0xF3; 
    
    apu->nr52 = 0xF1;
}


static uint16_t get_frequency(const APU* apu, int channel) {
    switch (channel) {
        case 1: return ((uint16_t)(apu->nr14 & 0x07) << 8) | apu->nr13;
        case 2: return ((uint16_t)(apu->nr24 & 0x07) << 8) | apu->nr23;
        case 3: return ((uint16_t)(apu->nr34 & 0x07) << 8) | apu->nr33;
    }
    return 0;
}


static void clock_sweep(APU *apu) {
    uint8_t sweep_period = (apu->nr10 >> 4) & 0x07;
    if (!apu->ch1_sweep_enabled || sweep_period == 0) {
        return;
    }

    if (--apu->ch1_sweep_timer <= 0) {
        apu->ch1_sweep_timer = sweep_period;
        
        uint16_t current_freq = apu->ch1_sweep_frequency;
        uint8_t shift = apu->nr10 & 0x07;
        uint16_t offset = current_freq >> shift;
        bool is_decrease = apu->nr10 & 0x08;
        
        uint16_t new_freq = is_decrease ? (current_freq - offset) : (current_freq + offset);

        if (new_freq > 2047) {
            apu->ch1_enabled = false;
            apu->nr52 &= ~0x01;
        } else if (shift > 0) {
            apu->ch1_sweep_frequency = new_freq;
            apu->nr13 = new_freq & 0xFF;
            apu->nr14 = (apu->nr14 & 0xF8) | ((new_freq >> 8) & 0x07);
        }
    }
}

static void trigger_ch1(APU *apu) {
    apu->ch1_enabled = true;
    // If DAC is off, channel is disabled immediately.
    if ((apu->nr12 & 0xF8) == 0) {
        apu->ch1_enabled = false;
    }

    // Reload length timer
    apu->ch1_length_timer = 64 - (apu->nr11 & 0x3F);

    // Reload envelope timer & volume
    apu->ch1_envelope_timer = apu->nr12 & 0x07;
    apu->ch1_envelope_volume = (apu->nr12 >> 4) & 0x0F;

    // Reload frequency timer (the actual timer value will be set in apu_step)
    apu->ch1_duty_pos = 0;
    uint16_t freq = get_frequency(apu, 1);
    apu->ch1_timer = (2048 - freq) * 4;

    // Initialize sweep
    apu->ch1_sweep_frequency = freq;
    apu->ch1_sweep_timer = (apu->nr10 >> 4) & 0x07;
    if (apu->ch1_sweep_timer == 0) apu->ch1_sweep_timer = 8; // A period of 0 is treated as 8
    
    apu->ch1_sweep_enabled = (apu->ch1_sweep_timer < 8 || (apu->nr10 & 0x07) != 0);
    // Perform initial sweep calculation
    if ((apu->nr10 & 0x07) != 0) {
        clock_sweep(apu); 
    }
}

static void trigger_ch2(APU *apu) {
    apu->ch2_enabled = true;
    if ((apu->nr22 & 0xF8) == 0) {
        apu->ch2_enabled = false;
    }
    apu->ch2_length_timer = 64 - (apu->nr21 & 0x3F);
    apu->ch2_envelope_timer = apu->nr22 & 0x07;
    apu->ch2_envelope_volume = (apu->nr22 >> 4) & 0x0F;
    apu->ch2_duty_pos = 0;
    uint16_t freq = get_frequency(apu, 2);
    apu->ch2_timer = (2048 - freq) * 4;
}

static void trigger_ch3(APU *apu) {
    apu->ch3_enabled = true;
    if ((apu->nr30 & 0x80) == 0) { // Check if DAC is enabled
        apu->ch3_enabled = false;
    }
    apu->ch3_length_timer = 256 - apu->nr31;
    apu->ch3_wave_pos = 0;
    uint16_t freq = get_frequency(apu, 3);
    apu->ch3_timer = (2048 - freq) * 2;
}

static void trigger_ch4(APU *apu) {
    apu->ch4_enabled = true;
    if ((apu->nr42 & 0xF8) == 0) {
        apu->ch4_enabled = false;
    }
    apu->ch4_length_timer = 64 - (apu->nr41 & 0x3F);
    apu->ch4_envelope_timer = apu->nr42 & 0x07;
    apu->ch4_envelope_volume = (apu->nr42 >> 4) & 0x0F;
    apu->ch4_lfsr = 0x7FFF; // Reset the LFSR
}


static void clock_length_counters(APU *apu) {
    // Channel 1
    if ((apu->nr14 & 0x40) && apu->ch1_length_timer > 0) {
        if (--apu->ch1_length_timer == 0) {
            apu->ch1_enabled = false;
            apu->nr52 &= ~0x01; // Clear CH1 status bit
        }
    }
    // Channel 2
    if ((apu->nr24 & 0x40) && apu->ch2_length_timer > 0) {
        if (--apu->ch2_length_timer == 0) {
            apu->ch2_enabled = false;
            apu->nr52 &= ~0x02; // Clear CH2 status bit
        }
    }
    // Channel 3
    if ((apu->nr34 & 0x40) && apu->ch3_length_timer > 0) {
        if (--apu->ch3_length_timer == 0) {
            apu->ch3_enabled = false;
            apu->nr52 &= ~0x04; // Clear CH3 status bit
        }
    }
    // Channel 4
    if ((apu->nr44 & 0x40) && apu->ch4_length_timer > 0) {
        if (--apu->ch4_length_timer == 0) {
            apu->ch4_enabled = false;
            apu->nr52 &= ~0x08; // Clear CH4 status bit
        }
    }
}

static void clock_volume_envelopes(APU *apu) {
    uint8_t ch1_period = apu->nr12 & 0x07;
    if (ch1_period > 0 && --apu->ch1_envelope_timer <= 0) {
        apu->ch1_envelope_timer = ch1_period;
        uint8_t current_vol = apu->ch1_envelope_volume;
        bool is_increase = apu->nr12 & 0x08;
        if (is_increase && current_vol < 15) apu->ch1_envelope_volume++;
        else if (!is_increase && current_vol > 0) apu->ch1_envelope_volume--;
    }

    uint8_t ch2_period = apu->nr22 & 0x07;
    if (ch2_period > 0 && --apu->ch2_envelope_timer <= 0) {
        apu->ch2_envelope_timer = ch2_period;
        uint8_t current_vol = apu->ch2_envelope_volume;
        bool is_increase = apu->nr22 & 0x08;
        if (is_increase && current_vol < 15) apu->ch2_envelope_volume++;
        else if (!is_increase && current_vol > 0) apu->ch2_envelope_volume--;
    }

    uint8_t ch4_period = apu->nr42 & 0x07;
    if (ch4_period > 0 && --apu->ch4_envelope_timer <= 0) {
        apu->ch4_envelope_timer = ch4_period;
        uint8_t current_vol = apu->ch4_envelope_volume;
        bool is_increase = apu->nr42 & 0x08;
        if (is_increase && current_vol < 15) apu->ch4_envelope_volume++;
        else if (!is_increase && current_vol > 0) apu->ch4_envelope_volume--;
    }
}

uint8_t apu_read(CPU *cpu, uint16_t addr){
    APU *apu = &cpu->apu;
    switch (addr) {
        // Channel 1
        case 0xFF10: return apu->nr10 | 0x80;
        case 0xFF11: return apu->nr11 | 0x3F;
        case 0xFF12: return apu->nr12;
        case 0xFF13: return 0xFF; // Write-only
        case 0xFF14: return apu->nr14 | 0xBF;
        // Channel 2
        case 0xFF16: return apu->nr21 | 0x3F;
        case 0xFF17: return apu->nr22;
        case 0xFF18: return 0xFF; // Write-only
        case 0xFF19: return apu->nr24 | 0xBF;
        // Channel 3
        case 0xFF1A: return apu->nr30 | 0x7F;
        case 0xFF1B: return 0xFF; // Write-only
        case 0xFF1C: return apu->nr32 | 0x9F;
        case 0xFF1D: return 0xFF; // Write-only
        case 0xFF1E: return apu->nr34 | 0xBF;
        // Channel 4
        case 0xFF20: return apu->nr41 | 0xC0;
        case 0xFF21: return apu->nr42;
        case 0xFF22: return apu->nr43;
        case 0xFF23: return apu->nr44 | 0xBF;
        // Control
        case 0xFF24: return apu->nr50;
        case 0xFF25: return apu->nr51;
        case 0xFF26: {
            uint8_t status = 0;
            if (apu->ch1_enabled) status |= 0x01;
            if (apu->ch2_enabled) status |= 0x02;
            if (apu->ch3_enabled) status |= 0x04;
            if (apu->ch4_enabled) status |= 0x08;
            return (apu->nr52 & 0xF0) | status | 0x70;
        }
        // Waveform RAM
        case 0xFF30: case 0xFF31: case 0xFF32: case 0xFF33:
        case 0xFF34: case 0xFF35: case 0xFF36: case 0xFF37:
        case 0xFF38: case 0xFF39: case 0xFF3A: case 0xFF3B:
        case 0xFF3C: case 0xFF3D: case 0xFF3E: case 0xFF3F:
            return apu->waveform[addr - 0xFF30];
        default:
             return 0xFF; 
    }
}

void apu_write(CPU *cpu, uint16_t addr, uint8_t value) {
    APU *apu = &cpu->apu;
    if (!(apu->nr52 & 0x80) && addr != 0xFF26) return;

    switch (addr) {
        // Channel 1
        case 0xFF10: apu->nr10 = value; break;
        case 0xFF11: apu->nr11 = value; break;
        case 0xFF12: apu->nr12 = value; break;
        case 0xFF13: apu->nr13 = value; break;
        case 0xFF14: apu->nr14 = value; if (value & 0x80) trigger_ch1(apu); break;
        // Channel 2
        case 0xFF16: apu->nr21 = value; break;
        case 0xFF17: apu->nr22 = value; break;
        case 0xFF18: apu->nr23 = value; break;
        case 0xFF19: apu->nr24 = value; if (value & 0x80) trigger_ch2(apu); break;
        // Channel 3
        case 0xFF1A: apu->nr30 = value; break;
        case 0xFF1B: apu->nr31 = value; break;
        case 0xFF1C: apu->nr32 = value; break;
        case 0xFF1D: apu->nr33 = value; break;
        case 0xFF1E: apu->nr34 = value; if (value & 0x80) trigger_ch3(apu); break;
        // Channel 4
        case 0xFF20: apu->nr41 = value; break;
        case 0xFF21: apu->nr42 = value; break;
        case 0xFF22: apu->nr43 = value; break;
        case 0xFF23: apu->nr44 = value; if (value & 0x80) trigger_ch4(apu); break;
        // Control
        case 0xFF24: apu->nr50 = value; break;
        case 0xFF25: apu->nr51 = value; break;
        case 0xFF26: apu->nr52 = (value & 0x80) | (apu->nr52 & 0x0F); break;
        // Waveform RAM
        case 0xFF30: case 0xFF31: case 0xFF32: case 0xFF33:
        case 0xFF34: case 0xFF35: case 0xFF36: case 0xFF37:
        case 0xFF38: case 0xFF39: case 0xFF3A: case 0xFF3B:
        case 0xFF3C: case 0xFF3D: case 0xFF3E: case 0xFF3F:
            apu->waveform[addr - 0xFF30] = value;
            break;
    }
}

// Clocks the length counters, volume envelopes, and frequency sweep
static void frame_sequencer(APU *apu) {
    switch (apu->frame_seq){
        case 0:
            clock_length_counters(apu);
            break;

        case 1:
            break;

        case 2:
            clock_length_counters(apu);
            clock_sweep(apu);
            break;

        case 3:
            break;

        case 4:
            clock_length_counters(apu);
            break;

        case 5:
            break;

        case 6:
            clock_length_counters(apu);
            clock_sweep(apu);
            break;

        case 7:
            clock_volume_envelopes(apu);
            break;
    }
}

// Duty cycle patterns for channels 1 and 2
static const int duty_patterns[4][8] = {
    {0, 0, 0, 0, 0, 0, 0, 1}, // 12.5%
    {1, 0, 0, 0, 0, 0, 0, 1}, // 25%
    {1, 0, 0, 0, 0, 1, 1, 1}, // 50%
    {0, 1, 1, 1, 1, 1, 1, 0}  // 75%
};

static int16_t get_ch1_output(const APU *apu) {
    // DAC must be enabled (top 5 bits of NR12 not zero)
    if (!apu->ch1_enabled || (apu->nr12 & 0xF8) == 0) {
        return 0;
    }
    uint8_t duty_index = (apu->nr11 >> 6) & 0x03;
    if (duty_patterns[duty_index][apu->ch1_duty_pos]) {
        return apu->ch1_envelope_volume;
    }
    return 0;
}

static int16_t get_ch2_output(const APU *apu) {
    if (!apu->ch2_enabled || (apu->nr22 & 0xF8) == 0) {
        return 0;
    }
    uint8_t duty_index = (apu->nr21 >> 6) & 0x03;
    if (duty_patterns[duty_index][apu->ch2_duty_pos]) {
        return apu->ch2_envelope_volume;
    }
    return 0;
}

static int16_t get_ch3_output(const APU *apu) {
    // Channel must be on (bit 7 of NR30)
    if (!apu->ch3_enabled || !(apu->nr30 & 0x80)) {
        return 0;
    }
    
    // Get the 4-bit sample from waveform RAM
    uint8_t sample_byte = apu->waveform[apu->ch3_wave_pos / 2];
    uint8_t sample = (apu->ch3_wave_pos % 2 == 0) ? (sample_byte >> 4) : (sample_byte & 0x0F);

    // Apply volume shift
    uint8_t volume_code = (apu->nr32 >> 5) & 0x03;
    switch (volume_code) {
        case 0: return 0;       // Muted
        case 1: return sample;  // 100%
        case 2: return sample >> 1; // 50%
        case 3: return sample >> 2; // 25%
    }
    return 0;
}

static int16_t get_ch4_output(const APU *apu) {
    if (!apu->ch4_enabled || (apu->nr42 & 0xF8) == 0) {
        return 0;
    }
    // Output is based on the inverse of the LFSR's lowest bit
    if (~apu->ch4_lfsr & 0x01) {
        return apu->ch4_envelope_volume;
    }
    return 0;
}

void apu_step(APU *apu, CPU *cpu){
    int cycles = cpu->cycles*4;
    for (int i = 0; i < cycles; ++i) {

        // Channel 1
        if (--apu->ch1_timer <= 0) {
            uint16_t freq = get_frequency(apu, 1);
            // The period of the timer is (2048 - frequency) * 4 CPU cycles
            apu->ch1_timer = (2048 - freq) * 4;
            apu->ch1_duty_pos = (apu->ch1_duty_pos + 1) % 8;
        }
        // Channel 2
        if (--apu->ch2_timer <= 0) {
            uint16_t freq = get_frequency(apu, 2);
            apu->ch2_timer = (2048 - freq) * 4;
            apu->ch2_duty_pos = (apu->ch2_duty_pos + 1) % 8;
        }
        // Channel 3
        if (--apu->ch3_timer <= 0) {
            uint16_t freq = get_frequency(apu, 3);
            // Period is (2048 - frequency) * 2 CPU cycles
            apu->ch3_timer = (2048 - freq) * 2;
            apu->ch3_wave_pos = (apu->ch3_wave_pos + 1) % 32;
        }
        // Channel 4
        if (--apu->ch4_timer <= 0) {
            uint8_t divisor_code = apu->nr43 & 0x07;
            int shift_amount = (apu->nr43 >> 4) & 0x0F;
            int divisor = (divisor_code == 0) ? 8 : (divisor_code * 16);
            apu->ch4_timer = divisor << shift_amount;

            // Clock the LFSR
            uint16_t xor_result = (apu->ch4_lfsr & 1) ^ ((apu->ch4_lfsr >> 1) & 1);
            apu->ch4_lfsr = (apu->ch4_lfsr >> 1) | (xor_result << 14);
            if (apu->nr43 & 0x08) { // 7-bit mode
                apu->ch4_lfsr &= ~(1 << 6);
                apu->ch4_lfsr |= (xor_result << 6);
            }
        }

        // The frame sequencer runs at 512 Hz, which is every 8192 CPU cycles.
        // It's responsible for clocking length, envelope, and sweep units.
        if (--apu->frame_seq_clock <= 0) {
            apu->frame_seq_clock = 8192; // Reset timer
            frame_sequencer(apu); // This is your existing function
            apu->frame_seq = (apu->frame_seq + 1) % 8;
        }

        if (--apu->main_clock <= 0) {
            apu->main_clock = CYCLES_PER_SAMPLE; // Reset sampler timer

            // Get the current output of each channel
            int16_t ch1_out = get_ch1_output(apu);
            int16_t ch2_out = get_ch2_output(apu);
            int16_t ch3_out = get_ch3_output(apu);
            int16_t ch4_out = get_ch4_output(apu);

            // Mix the channels. For simplicity, we'll do a basic sum.
            // A more advanced mixer would handle master volume (NR50) and panning (NR51).
            int16_t mixed_sample = (ch1_out + ch2_out + ch3_out + ch4_out) / 4;
            
            SDL_LockAudioDevice(audio_device);
            // Place the sample in the circular buffer if there's space
            int next_write_pos = (apu->write_pos + 1) % 4096;
            if (next_write_pos != apu->read_pos) {
                // A simple volume multiplier to make the sound audible
                apu->internal_buffer[apu->write_pos] = mixed_sample * 100;
                apu->write_pos = next_write_pos;
            }
            SDL_UnlockAudioDevice(audio_device);
        }
    }
}