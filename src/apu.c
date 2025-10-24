#include "cpu.h"
#include <string.h>
#include <math.h>

void apu_init(APU *apu) {
    
    memset(apu, 0, sizeof(APU));
    atomic_init(&apu->write_pos, 0);
    atomic_init(&apu->read_pos, 0);
    apu->sample_counter = 0.0;
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
    // Why are you even in this commit
    return 0xFF;
}

void apu_write(CPU *cpu, uint16_t addr, uint8_t value) {
    // Yes I have a lot  of work left, I am perfectly aware
}