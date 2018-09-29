#!/bin/bash
emmake make
mv dome dome.bc
emcc dome.bc -o test.html -s WASM=1 -s USE_SDL=2 -s ALLOW_MEMORY_GROWTH=1 --preload-file game.egg --preload-file res/around-the-corner.ogg --preload-file setup.sh --emrun -s "BINARYEN_TRAP_MODE='clamp'"
emrun test.html
