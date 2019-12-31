[< Back](.)

Installation
=================

You can either download the compiled binaries from GitHub, or you can build the DOME executable from source.

## Pre-requistes

You need to make sure a version of SDL2 is installed. On a Mac, you can either use a package manager like [Homebrew](https://brew.sh), or install the SDL2 framework manually from [here](https://www.libsdl.org/download-2.0.php). If you are going to build DOME from source, you will need the developer library.

On linux, you can use your package manager to install SDL2, or build `libSDL2.so` from source.

Windows is more difficult, and we recommend using the pre-compiled binaries for DOME, as it includes SDL2.

## Download

Go to the [Github](https://github.com/avivbeeri/dome/releases) release for the version of DOME you want, and select a zip file from the "assets" matching your computer's architecture to download.

Once it's downloaded to your computer, unzip it, and place it in the directory you want to make your game in.

At this point, you should have an executable named `dome` in your current directory. Test your installation by running:

```bash
> ./dome -v
```
It should print out its version, SDL versions and whether FFI is available (compiled binaries currently don't include FFI.)

You can also download the example `game.egg` from the GitHub repository to test dome. Place it in the same folder as your executable and then run dome, and it should begin playing.

## Build from Source

DOME should build fine on Mac OS X and most flavors of linux, so long as gcc, git and SDL2 are installed. 

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
