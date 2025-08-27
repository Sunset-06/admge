# admge
A simple emulator for the original GameBoy (and hopefully CGB too) in C.

Massive WIP, so this project will take some time to even reach playable state.

## Work left:

- Get all instructions done :white_check_mark:
- Get a game to boot :white_check_mark:
- Get a boootrom working :white_check_mark:
- Pass instruction tests :white_check_mark: 
- Handle inputs :white_check_mark:
- Pass PPU tests <--- [You are here]
- Handle audio
- Get bank switching working

  
> "You shouldn't implement 255 instructions at once. It becomes hell to debug later" <br/>
> _~someone who I should've encountered earlier in my life_

## Get it to Work
You need SDL2 to run this emulator.
then, use flag -libsdl2 when compiling. However due to how messy the directory is, I recommend using the provided makefile
```bash
make # compiles the emu into the /bin directory

make clean # deletes /bin and its contents 
```

Then to run it:
```bash
./bin/admge /path/to/your/rom.gb # run with the provided bootrom

./bin/admge /path/to/your/rom.gb -noboot # run without a bootrom

```


There's a bootrom in this repo, shamelessly sourced from [Hacktix](https://github.com/Hacktix/Bootix). If you want to use your own, change the path on **line 6** of `mem.c`