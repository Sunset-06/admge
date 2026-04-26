# admge
A simple emulator for the original GameBoy (and hopefully eventually CGB too) in C.

This was started as a way to play a specific game.
Since this was achieved, the project will only get intermittent updates depending on my free time.

## Work left:

- Get all instructions done :white_check_mark:
- Get a game to boot :white_check_mark:
- Get a boootrom working :white_check_mark:
- Pass blaarg :white_check_mark: 
- Handle inputs :white_check_mark:
- Pass acid2 :white_check_mark:
- Get bank switching/ MBC working :white_check_mark:
- Handle audio :white_check_mark:
- Fix MBCs <--- [You are here]
- Pass Mooneye tests
- Add a GUI (I'll do this whenever I feel like it honestly)
- Save States
## Issues:

Note: I'm relying on Vsync to limit the framerate. While untested, this should break on monitors with refresh rate above 60Hz.  
What if you've disabled VSync for your graphics driver? Well, enjoy the raw speed of your CPU then :D

For now just use VSync with a 60Hz refresh rate  
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

Also, you need to pray (to your preferred deity) that the rom you selected runs properly. Consider this a formal Step 3.

The border assets used in this repo are sourced from the [BGB Reality page](https://bgb.bircd.org/reality/index.html)

If you wish to use a bootrom with this, put it into /bootrom in the root of the project.
I recommend using [Hacktix](https://github.com/Hacktix/Bootix).