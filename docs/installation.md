Installation
=================

At the moment, the DOME executable needs to be built from source.

DOME should build fine on Mac OS X and most flavors of linux, so long as gcc, git and SDL2 are installed. DOME can be built on Windows, but it's only been tested using a MinGW-w64 MSYS2 environment.

1. You can clone the repository by doing the following in your terminal:

```
> git clone https://github.com/avivbeeri/dome.git
```

2. Initialise the sub-modules DOME depends on (Wren):

```
> cd dome
> git submodule init
> git submodule update
```

3. Build using GNU Make:

```
> make
```

At this point, you should have an executable named `dome` in your current directory. Test your installation by running:

```
> ./dome
```

If all is well, you should see the example game:

![Image of Tutorial Shmup](https://octodex.github.com/assets/shmup.png)
