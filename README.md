# admge
A simple emulator for the original GameBoy (and hopefully CGB too) in C.

Massive WIP, so this project will take time.

So far, I'm focusing on getting all instructions working.
The current processor just uses two massive switch statements.
I've used this as opposed to an array of function pointers simply because I don't really see much benefit to that approach. This should be comparitively straightforward to debug and I doubt it will affect the speed, as the compiler has to spend time dereferencing anyway.  

Once it reaches a point where I can test it, i'll use the approach that ends up being faster.


There's a bootrom in this repo, shamelessly sourced from [Hacktix](https://github.com/Hacktix/Bootix) 