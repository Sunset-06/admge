# admge
A simple emulator for the original GameBoy (and hopefully CGB too) in C.

Massive WIP, so this project ~~will take some time to even reach playable state~~ can run MBC1 games now!

Audio support should come soon!

## Work left:

- Get all instructions done :white_check_mark:
- Get a game to boot :white_check_mark:
- Get a boootrom working :white_check_mark:
- Pass instruction tests :white_check_mark: 
- Handle inputs :white_check_mark:
- Pass PPU tests :white_check_mark:
- Get bank switching/ MBC working :white_check_mark:
- Handle audio :white_check_mark:
- Implement MBC2 and onwards <--- [You are here]
- Overhaul the PPU <--- [and also here] 
- Add a GUI

(updates will be less frequent and scattered now)

## Issues:

There is a ppu overhaul being worked on the ppu/accurate branch. This will however take some time, and won't be merged till I am convinced that it is a better implementation.

Note: I'm relying on Vsync to limit the framerate. While untested, this should break on monitors with refresh rate above 60Hz.  
What if you've disabled VSync for your graphics driver? Well, enjoy the raw speed of your CPU then :D

I'm actually not sure how to handle this issue, I could potentially use `sleep()` or other equivalents.  
(I do not want to do that as that will bring about many more issues, and honestly, it feels a bit lazy)

So for now just use VSync with a 60Hz refresh rate  
<img src="https://i.pinimg.com/474x/3b/bb/db/3bbbdbca9e30c5dc52b069320aa54ab7.jpg" height="60" width="60" style="vertical-align: middle;" />

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