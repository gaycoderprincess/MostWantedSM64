# Most Wanted SM64

A mod for Need for Speed: Most Wanted that adds Mario from Super Mario 64

## Installation

- Make sure you have v1.3 of the game, as this is the only version this plugin is compatible with. (exe size of 6029312 bytes)
- Plop the files into your game folder, and place a ROM of the US version of Super Mario 64 next to the files, renamed `baserom.us.z64`.
- Enjoy, nya~ :3

## Disclaimer

Due to the current programming landscape, I feel that it's necessary to explicitly state that this project had zero assistance or any other kind of involvement from any sort of "AI agent" and it never will.  
This mod was entirely built by hand, by a human being, and I believe that any project that cannot also claim this about itself is not worth people's time.

## Features

- Mario uses XInput bindings, with A bound to A, B bound to B, and Z bound to X/LB/RB
- On keyboard, use the arrow keys, space, left control and the left mouse button to control Mario
- You can resize Mario by editing the world_scale value in `NFSMWSM64_gcp.toml`

## Known issues

- Mario makes skidmarks sometimes
- Since you can jump really high now, it's trivially easy to overshoot the checkpoints in races
- Mario is bound to your car's physics model, therefore collisions with other cars and smackable objects can be wonky at times

## Building

Building is done on an Arch Linux system with CLion being used for the build process. 

Before you begin, clone [nya-common](https://github.com/gaycoderprincess/nya-common), [nya-common-nfsmw](https://github.com/gaycoderprincess/nya-common-nfsmw), and [CwoeeMenuLib](https://github.com/gaycoderprincess/CwoeeMenuLib) to folders next to this one, so they can be found.

Required packages: `mingw-w64-gcc vcpkg`

To install all dependencies, use:
```console
vcpkg install tomlplusplus:x86-mingw-static
```

To install the BASS audio library:

Download the Win32 version from [here](https://www.un4seen.com/bass.html) and extract it somewhere

Once that's done, copy `bass.lib` from the `c` folder into `nya-common/lib32`

You should be able to build the project now in CLion.
