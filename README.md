# admge
A simple and (I like to think) lightweight emulator for the original GameBoy in C.

This was started as a way to play a specific game and has reached a baseline level. From here on, the project will only get intermittent updates depending on my free time.

## Controls



## Issues:

The emulator is only minimally accurate, some games do not work perfectly, and may have minor issues. However, all blaarg tests pass, so it should support most games.

It isn't completely cycle accurate (The cycles get added after the instructions are carried out).

Note: I'm relying on Vsync to limit the framerate. While untested, this should break on monitors with refresh rate above 60Hz.  
What if you've disabled VSync for your graphics driver? Well, enjoy the raw speed of your CPU then :D

For now just use VSync with a 60Hz refresh rate  
<img src="https://i.pinimg.com/474x/3b/bb/db/3bbbdbca9e30c5dc52b069320aa54ab7.jpg" height="60" width="60" style="vertical-align: middle;" />

> "You shouldn't implement 255 instructions at once. It becomes hell to debug later" <br/>
> _~someone who I should've encountered earlier in my life_

## Get it to Work
You need SDL2 and SDL_Image to run this emulator.
then, use flag -libsdl2 when compiling. However due to how messy the directory is, I recommend using the provided makefile
```bash
make # compiles the emu into the /bin directory

make clean # deletes /bin and its contents 
```

Then to run it:
```bash
./bin/admge /path/to/your/rom.gb # run with the provided bootrom

./bin/admge /path/to/your/rom.gb -noboot # run without a bootrom

./bin/admge /path/to/your/rom.gb -test # run in headless mode

./bin/admge /path/to/your/rom.gb -mgb # run in mgb mode (only cosmetic)

./bin/admge # start through UI
```

Also, you need to pray (to your preferred deity) that the rom you selected runs properly. Consider this a formal Step 3.

The border assets used in this repo are sourced from the [BGB Reality page](https://bgb.bircd.org/reality/index.html)

If you wish to use a bootrom with this, put it into /bootrom in the root of the project.
I recommend using [Hacktix](https://github.com/Hacktix/Bootix).

By default, the emulator looks for `/bootrom/boot.bin` Ensure this file exists to use a bootrom.

## Test it
During development, the following test roms were used:

[Blaarg's Gameboy Test Roms](https://github.com/retrio/gb-test-roms/)

[dmg-acid2](https://github.com/mattcurrie/dmg-acid2)

[mooneye test suite](https://github.com/Gekkio/mooneye-test-suite/)

[MBC3 RTC test roms](https://github.com/aaaaaa123456789/rtc3test/)