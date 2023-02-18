# DOME - Design-Oriented Minimalist Engine

A comfortable framework for game development which melds SDL2 and the [Wren scripting language](http://wren.io), written in C.

![Image of DOME logo](https://domeengine.com/assets/logo200.png)

### For more information on how to use DOME and get started, read the docs [here](https://domeengine.com).

## How to Use

### Download

You can download production-ready binaries from our [Releases page](https://github.com/domeengine/dome/releases/latest). This is the recommended method for distribution and easy development.

### Install via Brew

Alternatively, if you have Homebrew installed (Mac OS X, Linux and WSL), you can install DOME using the following commands:

```bash
> brew tap domeengine/tap
> brew install dome
```

### Build

Finally, if you want to build DOME yourself, to make modifications or other reasons, follow these instruction instead.

Ensure you have the shared SDL2 libraries installed on your system first, then to build, run:

```bash
> make
```

This will create an executable named `./dome` (on Mac OS X and Linux), and `./dome-x32.exe` or `./dome-x64.exe`.

### Run

Run `./dome [gamefile.wren]` to run your game. If your initial file is called `main.wren`, just running `./dome` will execute it. Replace `dome` with your built binary name as necessary.

## Basics

Your game's entry point must contain a `Game` variable which contains at least `init()`, `update()` and `draw(_)` methods.

```wren
import "input" for Keyboard
import "graphics" for Canvas, Color

class Main {
  construct new() {}

  init() {
    _x = 10
    _y = 10
    _w = 5
    _h = 5
  }

  update() {
    if (Keyboard.isKeyDown("left")) {
      _x = _x - 1
    }
    if (Keyboard.isKeyDown("right")) {
      _x = _x+ 1
    }
    if (Keyboard.isKeyDown("up")) {
      _y = _y - 1
    }
    if (Keyboard.isKeyDown("down")) {
      _y = _y + 1
    }
  }

  draw(alpha) {
    Canvas.cls()
    var color = Color.rgb(171, 82, 54)
    Canvas.rectfill(_x, _y, _w, _h, color)
  }
}

var Game = Main.new()
```

## Modules

DOME provides the following features, and more:

- Graphics
  - Canvas
    - Rect
    - Point
    - Circle
    - Ellipses
    - Lines
    - Triangles
  - Color
  - ImageData (aka Bitmap)
    - Draw sprites loaded from files (png)
  - SpriteSheet support
- Input
  - Keyboard
  - Mouse
  - Gamepads
- Filesystem
  - File reading and writing
- Audio (stereo and mono OGG, MP3, FLAC and WAV files)
- Collections (abstact types)
  - Set
  - Queue
  - Stack
  - Priority Queue
- Native Plugins (allowing access to all kinds of functionality!)

## TODO

You can follow my progress on implementing DOME on [my twitter](https://twitter.com/avivbeeri/status/1012448692119457798).

- Graphics
  - Potential 3D rendering mode?
- IO
  - Asynchronous Operations
- Network Access
  - UDP
  - HTTP client (maybe)
- Security sandboxing (maybe)

## Dependencies

DOME currently depends on a few libraries to achieve it's functions.

- Wren (included in the project repo already)
- SDL2 (version 2.0.12 or newer. If you install this from source, you'll want to build shared/dynamic libraries.)
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
- mkdirp
- whereami
- dr_mp3
- dr_flac


Apart from SDL2, all other dependancies are baked in. DOME aspires to be both minimalist and cross platform, so it depends on as few external components as possible.

## Acknowledgements

- Bob Nystrom for creating Wren and inspiring me to make games, thanks to [Game Programming Patterns](http://gameprogrammingpatterns.com)
- Special thanks to [lqdev](https://github.com/liquid600pgm) for the fantastic logo!
- Glenn Fiedler for the most referenced [resources](https://gafferongames.com/) on Game Loops, Physics and Networking in games
- Casey Muratori for creating [Handmade Hero](https://hero.handmade.network), an inspiration and educational resource that makes this project possible.
- Built-in font comes from [here](https://github.com/dhepper/font8x8).
- Sean Barrett for [multiple libraries](https://github.com/nothings/stb)
- rxi for [microtar](https://github.com/rxi/microtar)
- Neil Henning for [utf8.h](https://github.com/sheredom/utf8.h)
- Chris Wellons for [optparse](https://github.com/skeeto/optparse) and [pdjson](https://github.com/skeeto/pdjson)
- cxong for [tinydir](https://github.com/cxong/tinydir)
- Jon Olick for [jo_gif](https://www.jonolick.com/home/gif-writer)
- Stephen Mathieson for [mkdirp](https://github.com/stephenmathieson/mkdirp.c)
- Gregory Pakosz for [whereami](https://github.com/gpakosz/whereami)
- David Reid for [dr_flac and dr_mp3](https://github.com/mackron/dr_libs)


### Example Game Resources

- Example game and graphics are derived from [this](https://ztiromoritz.github.io/pico-8-shooter/) fantastic PICO-8 tutorial.
- Aerith's Piano Theme (res/AerisPiano.ogg) by Tanner Helland is available under a CC BY-SA 3.0 license: [Link](http://www.tannerhelland.com/68/aeris-theme-piano/)
- Game Over Theme (res/music.wav) by Doppelganger is available under a CC BY-SA 3.0 license: [Link](https://opengameart.org/content/game-over-theme)
- Font "Memory" is provided by Eeve Somepx, and is available on their patreon [here](https://www.patreon.com/posts/free-font-memory-28150678) under a [common sense license](http://www.palmentieri.it/somepx/license.txt).
