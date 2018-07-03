# DOME - Dynamic Opinionated Mini Engine

A lightweight game engine which melds SDL2 and the Wren scripting language, written in C.

## How to Use

### Build

Execute `make` and it will first pull and build the Wren VM if it hasn't been, and then the DOME core.

### Run

Run `./dome [gamefile.wren]` to run your game.

## Basics

Your game's entry point must contain a `Game` class which contains at least `static init()`, `static update()` and `static draw(_)` methods.

```wren
import "input" for Keyboard
import "graphics" for Graphics, Color

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
    Graphics.cls()
    var color = Color.new(171, 82, 54).rgb
    Graphics.rectfill(__x, __y, __w, __h, color)
  }
}

```

## Modules

DOME provides the following modules/methods/classes:
- Graphics
  - Color
  - Rectfill
  - Point
- Input

## TODO

- IO
- Audio
- Math


