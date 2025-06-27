# CGB-Emu
A simple emulator for the original GameBoy (and hopefully CGB too) in C.

The current interpreter just uses one massive switch statement.

I've used this as opposed to an array of function pointers simply because I don't really see much benefit to that approach. This should be compritively straightforward to debug and I doubt it will affect the speed, simply because the compiler has to spend time dereferencing anyway.  

Once it reaches a point where I can test it, i'll use the approach that ends up being faster, including a third approach that might be better.

For now, all the instructions are in `operations.c`.