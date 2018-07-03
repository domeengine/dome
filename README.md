# DOME - Dynamic Opinionated Mini Engine

A lightweight game engine which melds SDL2 and the Wren scripting language, written in C.

## How to Use

### Build

Execute `make` and it will first pull and build the Wren VM if it hasn't been, and then the DOME core.

### Run

Run `./dome [gamefile.wren]` to run your game.

## Basics

Your game's entry point must contain a `Game` class which contains at least `init()`, `update()` and `draw(_)` methods.

```

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


