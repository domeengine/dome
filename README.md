# DOME - Design-Oriented Minimalist Engine

A comfortable framework for game development which melds SDL2 and the [Wren scripting language](http://wren.io), written in C.

![Image of DOME logo](https://avivbeeri.github.com/dome/assets/logo200.png)

### For more information on how to use DOME and get started, read the docs [here](https://domeengine.com).

## How to Use

### Download

You can download production-ready binaries from our [Releases page](https://github.com/avivbeeri/dome/releases/latest)

### Build

If you want to build DOME yourself, to make modifications or other reasons, follow these instruction instead.

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
  static draw(alpha) {
    Canvas.cls()
    var color = Color.rgb(171, 82, 54)
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

- Graphics 
  - Triangles
- IO
  - Asynchronous Operations
  - Audio and Graphics also
- Network Access
  - UDP
  - HTTP client (maybe)
- Security sandboxing (maybe)

## Dependencies

DOME currently depends on a few libraries to achieve it's functions.
- Wren (This is built by `make` automatically)
- SDL2 (version 2.0.2 or newer, this is a shared library and you must install it seperately)
- libffi (version 3.3 or newer, but optional and can be built by `make DOME_OPT_FFI=1`)
- utf8.h
- stb_image
- stb_image_write
- stb_truetype
- stb_vorbis
- microtar
- optparse
- jo_gif
- tinydir
- [ABC_fifo](https://github.com/avivbeeri/abc) (A SPMC threadpool/task dispatching FIFO I wrote for this project)

Apart from SDL2, all other dependancies are baked in or linked statically. DOME aspires to be both minimalist and cross platform, so it depends on as few external components as possible.

## Acknowledgements

- Bob Nystrom for creating Wren and inspiring me to make games, thanks to [Game Programming Patterns](http://gameprogrammingpatterns.com)
- Special thanks to [lqdev](https://github.com/liquid600pgm) for the fantastic logo!
- Glenn Fiedler for the most referenced [resources](https://gafferongames.com/) on Game Loops, Physics and Networking in games
- Casey Muratori for creating [Handmade Hero](https://hero.handmade.network), an inspiration and educational resource that makes this project possible. 
- Built-in font comes from [here](https://github.com/dhepper/font8x8).
- Sean Barrett for [multiple libraries](https://github.com/nothings/stb)
- rxi for [microtar](https://github.com/rxi/microtar)
- Neil Henning for [utf8.h](https://github.com/sheredom/utf8.h)
- Chris Wellons for [optparse](https://github.com/skeeto/optparse)
- cxong for [tinydir](https://github.com/cxong/tinydir)
- Jon Olick for [jo_gif](https://www.jonolick.com/home/gif-writer)

### Example Game Resources
- Example game and graphics are derived from [this](https://ztiromoritz.github.io/pico-8-shooter/) fantastic PICO-8 tutorial.
- Aerith's Piano Theme (res/AerisPiano.ogg) by Tanner Helland is available under a CC BY-SA 3.0 license: [Link](http://www.tannerhelland.com/68/aeris-theme-piano/)
- Game Over Theme (res/music.wav) by Doppelganger is available under a CC BY-SA 3.0 license: [Link](https://opengameart.org/content/game-over-theme)
- Font "Memory" is provided by Eeve Somepx, and is available on their patreon [here](https://www.patreon.com/posts/free-font-memory-28150678) under a [common sense license](http://www.palmentieri.it/somepx/license.txt).
