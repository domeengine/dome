#!/bin/bash
DOME_DIR=$PWD
INCLUDE_DIR=$DOME_DIR/include
LIB_DIR=$DOME_DIR/lib
OBJ_DIR=$DOME_DIR/obj
WREN_DIR=$LIB_DIR/wren

cd "$WREN_DIR/util"
./generate_amalgamation.py > "$LIB_DIR/wren_all.c"
cd "$DOME_DIR"
emcc -c "$LIB_DIR/wren_all.c" -o "$OBJ_DIR/web/wren.o"
cp "$WREN_DIR/src/include/wren.h" "$INCLUDE_DIR/wren.h"
