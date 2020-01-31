[< Back](.)

Installation
=================

You can either download the compiled binaries from GitHub, or you can build the DOME executable from source.

## Method 1: Download

Go to the [Github releases page](https://github.com/avivbeeri/dome/releases/v1.0.0) and select a zip file from the "assets" matching your computer's operating system and architecture to download.

Once the download is complete, unzip the downloaded archive and place its contents in the directory you want to make your game in.

At this point, you should have an executable named `dome` in your current directory. Test your installation by opening a terminal/command-prompt, navigating to the directory and executing:

```bash
> ./dome -v
```
It should print out its version, SDL versions and whether FFI is available (compiled binaries currently don't include FFI.)

You can also download the example `game.egg` from the GitHub repository to test dome. Place it in the same folder as your executable and then run dome, and it should begin playing.

## Method 2: Build from Source

DOME should build on most unix-like platforms, so long as gcc, git and SDL2 are installed. If these are installed, you can skip to the [Compilation](#compilation) step below.


### Pre-requisite: SDL2

On Mac OS X, you can install SDL2 by using [Homebrew](https://brew.sh) via the command `brew install sdl2`, or installing the official SDL2.framework file from [the SDL2 website](https://www.libsdl.org/download-2.0.php). Finally, you can also compile SDL2 from source and install it "the unix way".

Windows is supported, but it's only been tested using a MinGW-w64 MSYS2 environment. For information on setting this up, follow [this guide.](https://github.com/orlp/dev-on-windows/wiki/Installing-GCC--&-MSYS2). You will need a version of SDL2 which is prepared specifically for MinGW-w64.

To compile on Linux, you need to either build SDL2 from source "the unix way", or install a pre-prepared package appropriate to your distribution.

### Compilation

Once the prerequisites are met, you can clone the DOME repository and build an executable binary by doing the following in your terminal:

```
> git clone https://github.com/avivbeeri/dome.git
> cd dome
> make
```

At this point, you should have an executable named `dome` in your current directory. Test your installation by running:

```
> ./dome examples/spaceshooter
```

If all is well, you should see the example game:

![Image of Tutorial Shmup](https://domeengine.com/assets/shmup.png)
