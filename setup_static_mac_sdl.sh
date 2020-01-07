#!/bin/bash
DOME_DIR=$PWD/src
DIRECTORY=$PWD/src/lib/SDL2-2.0.2

if ! [ -d "$DIRECTORY" ]; then
  cd $DOME_DIR/lib
  curl -L https://libsdl.org/release/SDL2-2.0.2.tar.gz | tar -xvz
  cd $DOME_DIR/lib/SDL2-2.0.2
  mkdir build
  cd build 
else
  cd $DOME_DIR/lib/SDL2-2.0.2/build
fi

mkdir -p $DOME_DIR/lib/SDL2
echo $PWD
CC=$DOME_DIR/lib/SDL2-2.0.2/build-scripts/gcc-fat.sh ../configure --prefix=$DOME_DIR/lib/SDL2
make && make install
ls -la $DOME_DIR/lib/SDL2

SDL_DIR=$DOME_DIR/lib/SDL2

cp $SDL_DIR/lib/libSDL2main.a $DOME_DIR/lib
cp $SDL_DIR/lib/libSDL2.a $DOME_DIR/lib

