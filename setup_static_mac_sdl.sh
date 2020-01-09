#!/bin/bash
DOME_DIR=$PWD/src
DIRECTORY=$PWD/src/lib/SDL2-2.0.2

if ! [ -d "$DIRECTORY" ]; then
  cd $DOME_DIR/lib
  curl -L https://libsdl.org/release/SDL2-2.0.2.tar.gz | tar -xvz
elif ! [ -d "$DIRECTORY/special" ]; then
  cd $DIRECTORY
  mkdir special ; cd special
  CC=$DIRECTORY/build-scripts/gcc-fat.sh ../configure 
else
  cd $DIRECTORY/special
fi

make

if [ -f "$DIRECTORY/special/build/.libs/libSDL2main.a" ]; then
  cp $DIRECTORY/special/build/.libs/libSDL2main.a $DOME_DIR/lib
fi

cp $DIRECTORY/special/build/.libs/libSDL2.a $DOME_DIR/lib
cp $DIRECTORY/special/sdl2-config $DOME_DIR/lib/sdl2-config

cp -r $DIRECTORY/include $DOME_DIR/include/SDL2
cp -r $DIRECTORY/special/include/SDL_config.h  $DOME_DIR/include/SDL2/SDL_config.h
