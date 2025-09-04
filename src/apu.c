/*
My APU needs to do these every step:

Advancing the internal state of the four APU channels based on how many cycles have passed.

Checking the Frame Sequencer to see if length, envelope, or sweep timers need to be clocked.

Generating audio samples at the correct rate (4,194,304 CPU cycles / 44,100 samples per second ≈ 95 cycles per sample).

Mixing the output of the four channels together.

Placing these generated samples into a separate, intermediate buffer*/


/* There will be a separate thread that consumes this buffer. This thread will run synced to real time NOT the cpu cycles. 
   I'll be placing this in my SDL_audio.c*/

