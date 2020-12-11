#!/bin/bash
DOME_DIR=$PWD
INCLUDE_DIR=$DOME_DIR/include
LIB_DIR=$DOME_DIR/lib
WREN_DIR=$LIB_DIR/wren

cd $WREN_DIR/projects

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  # ...
  cd make
elif [[ "$OSTYPE" == "darwin"* ]]; then
  cd make.mac
  # Mac OSX
elif [[ "$OSTYPE" == "msys" ]]; then
  cd make
  # Lightweight shell and GNU utilities compiled for Windows (part of MinGW)
elif [[ "$OSTYPE" == "win32" ]]; then
  exit 1
else
  exit 1
fi

# Undo external makefile flags just in case
unset config
MAKEFLAGS="--no-print-directory"
echo $config
echo ${@:2}
# build the debug version of wren
make clean
make ${@:2} verbose=1 config=debug_$1 wren && cp $WREN_DIR/lib/libwren_d.a $LIB_DIR/libwrend.a
# build the release version of wren
make clean
make ${@:2} verbose=1 config=release_$1 wren && cp $WREN_DIR/lib/libwren.a $LIB_DIR/libwren.a
# Copy the wren.h to our includes
cp $WREN_DIR/src/include/wren.h $INCLUDE_DIR/wren.h
