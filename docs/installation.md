[< Back](.)

Installation
=================

You can either download the compiled binaries from GitHub, or you can build the DOME executable from source.

## Method 1: Download

Go to the [Github](https://github.com/avivbeeri/dome/releases) release for the version of DOME you want, and select a zip file from the "assets" matching your computer's architecture to download.

Once it's downloaded to your computer, unzip it, and place it in the directory you want to make your game in.
For linux and Mac OS X, you will need a version of SDL2 installed as well.

At this point, you should have an executable named `dome` in your current directory. Test your installation by running:

```bash
> ./dome -v
```
It should print out its version, SDL versions and whether FFI is available (compiled binaries currently don't include FFI.)

You can also download the example `game.egg` from the GitHub repository to test dome. Place it in the same folder as your executable and then run dome, and it should begin playing.

## Method 2: Build from Source

DOME should build fine on Mac OS X and most flavors of linux, so long as gcc, git and SDL2 are installed. On Mac OS X, you should install SDL2 using [Homebrew](https://brew.sh) using the command `brew install sdl2`.

Windows is supported, but it's only been tested using a MinGW-w64 MSYS2 environment. For information on setting this up, follow [this guide.](https://github.com/orlp/dev-on-windows/wiki/Installing-GCC--&-MSYS2)

You can clone the repository and build DOME by doing the following in your terminal:

```
> git clone https://github.com/avivbeeri/dome.git
> cd dome
> make
```

At this point, you should have an executable named `dome` in your current directory. Test your installation by running:

```
> ./dome
```

If all is well, you should see the example game:

![Image of Tutorial Shmup](https://avivbeeri.github.com/dome/assets/shmup.png)
