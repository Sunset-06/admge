#include "cpu.h"

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


void apu_step(APU *apu, CPU *cpu){

}