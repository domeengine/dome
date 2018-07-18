# DOME - Dynamic Opinionated Mini Engine

A lightweight game engine which melds SDL2 and the [Wren scripting language](http://wren.io), written in C.

## Warning

As of 03/07/2018, DOME is in a pre-alpha state. None of the API is stable and it is not production ready. 

## How to Use

### Build

```bash
> git submodule update
> make
```

### Run

Run `./dome [gamefile.wren]` to run your game.

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
  - Color
  - Rectfill
  - Point
  - Circle
  - Lines
- ImageData
  - Draw sprites loaded from files
- Input
- Audio (stereo and mono OGG and WAV files only)
- Random

## TODO

- IO
- Graphics 
  - Triangles
- Network Access
- Math
- Robust error checking
- Documentation

## Dependencies

DOME currently depends on a few libraries to achieve it's functions.
- Wren (This is built by `make` automatically)
- SDL2

## Acknowledgements

- Bob Nystrom for creating Wren and inspiring me to make games, thanks to [Game Programming Patterns](http://gameprogrammingpatterns.com)
- Font comes from [here](https://opengameart.org/content/ascii-bitmap-font-cellphone)
- Sean Barrett for [multiple libraries](https://github.com/nothings/stb)
- Example game and graphics are derived from [this](https://ztiromoritz.github.io/pico-8-shooter/) fantastic PICO-8 tutorial.
- Aerith's Piano Theme (res/AerisPiano.ogg) by Tanner Helland is available under a CC BY-SA 3.0 license: [Link](http://www.tannerhelland.com/68/aeris-theme-piano/)
- Game Over Theme (res/music.wav) by Doppelganger is available under a CC BY-SA 3.0 license: [Link](https://opengameart.org/content/game-over-theme)
