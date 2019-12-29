# DOME - Dynamic Opinionated Mini Engine

A lightweight game framework which melds SDL2 and the [Wren scripting language](http://wren.io), written in C.

### For more information on how to use DOME and get started, read the docs [here](https://avivbeeri.github.io/dome).

## API Stability

As of 24/12/2019, DOME is approaching a beta state. APIs which are found in the docs can be considered stable. It has been tested in OSX Sierra and 64-bit Lubuntu 18.04, and can be compiled on Windows 10 using MinGW-w64 and MSYS2.

## How to Use

### Build

Ensure you have the shared SDL2 libraries installed on your system first, then to build, run:

```bash
> make
```

### Run

Run `./dome [gamefile.wren]` to run your game. If your initial file is called `main.wren`, just running `./dome` will execute it.

## Basics

Your game's entry point must contain a `Game` class which contains at least `static init()`, `static update()` and `static draw(_)` methods.

```wren
import "input" for Keyboard
import "graphics" for Canvas, Color

class Game {

  static init() {
    __x = 10
    __y = 10
    __w = 5
    __h = 5
  }

  static update() {
    if (Keyboard.isKeyDown("left")) {
      __x = __x - 1 
    }
    if (Keyboard.isKeyDown("right")) {
      __x = __x+ 1 
    }
    if (Keyboard.isKeyDown("up")) {
      __y = __y - 1 
    }
    if (Keyboard.isKeyDown("down")) {
      __y = __y + 1 
    }
  }
  static draw(dt) {
    Canvas.cls()
    var color = Color.new(171, 82, 54)
    Canvas.rectfill(__x, __y, __w, __h, color)
  }
}

```

## Modules

DOME provides the following modules/methods/classes:
- Graphics
  - Canvas
    - Rect
    - Point
    - Circle
    - Lines
  - Color
  - ImageData
    - Draw sprites loaded from files (png)
- Input
  - Keyboard
  - Mouse
  - Gamepads
- Filesystem
  - File reading and writing
- Audio (stereo and mono OGG and WAV files only)

## TODO
You can follow my progress on implementing DOME on [my twitter](https://twitter.com/avivbeeri/status/1012448692119457798).

- IO
  - Asynchronous Operations (Unstable API)
- Loading Audio and Graphics asynchronously
- Graphics 
  - Triangles
- Network Access
  - UDP
  - HTTP client (optional)
- Math (More intuitive API for Num class functions)
- Robust error checking and sandboxing

## Dependencies

DOME currently depends on a few libraries to achieve it's functions.
- Wren (This is built by `make` automatically)
- SDL2 (version 2.0.4 or newer, this is a shared library and you must install it seperately)
- libffi (version 3.3 or newer, but optional and can be built by `make DOME_OPT_FFI=1`)
- stb_image
- stb_image_write
- stb_vorbis
- microtar
- optparse
- jo_gif
- [ABC_fifo](https://github.com/avivbeeri/abc) (A SPMC threadpool/task dispatching FIFO I wrote for this project)

Apart from SDL2, all other dependancies are baked in or linked statically. DOME aspires to be both minimalist and cross platform, so it depends on as few external components as possible.

## Acknowledgements

- Bob Nystrom for creating Wren and inspiring me to make games, thanks to [Game Programming Patterns](http://gameprogrammingpatterns.com)
- Glenn Fiedler for the most referenced [resources](https://gafferongames.com/) on Game Loops, Physics and Networking in games
- Casey Muratori for creating [Handmade Hero](https://hero.handmade.network), an inspiration and educational resource that makes this project possible. 
- Font comes from [here](https://opengameart.org/content/ascii-bitmap-font-cellphone) and [here](https://github.com/dhepper/font8x8).
- Sean Barrett for [multiple libraries](https://github.com/nothings/stb)
- rxi for [microtar](https://github.com/rxi/microtar)
- Jon Olick for [jo_gif](https://www.jonolick.com/home/gif-writer)
- Chris Wellons for [optparse](https://github.com/skeeto/optparse)

### Example Game Resources
- Example game and graphics are derived from [this](https://ztiromoritz.github.io/pico-8-shooter/) fantastic PICO-8 tutorial.
- Aerith's Piano Theme (res/AerisPiano.ogg) by Tanner Helland is available under a CC BY-SA 3.0 license: [Link](http://www.tannerhelland.com/68/aeris-theme-piano/)
- Game Over Theme (res/music.wav) by Doppelganger is available under a CC BY-SA 3.0 license: [Link](https://opengameart.org/content/game-over-theme)
