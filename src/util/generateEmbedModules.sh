#!/bin/bash
gcc embed.c -o embed -std=c99
./embed ../modules/dome.wren domeModule ../modules/dome.wren.inc
./embed ../modules/input.wren inputModule ../modules/input.wren.inc
./embed ../modules/graphics.wren graphicsModule ../modules/graphics.wren.inc
./embed ../modules/io.wren ioModule ../modules/io.wren.inc
./embed ../modules/audio.wren audioModule ../modules/audio.wren.inc
./embed ../modules/point.wren pointModule ../modules/point.wren.inc
