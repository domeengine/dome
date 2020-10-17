#!/bin/bash
VERSION=2.0.12
FOLDER=special
DOME_DIR=$PWD/src
DIRECTORY=$PWD/src/lib/SDL2-${VERSION}

if ! [ -d "$DIRECTORY" ]; then
  cd $DOME_DIR/lib
  curl -L https://libsdl.org/release/SDL2-${VERSION}.tar.gz | tar -xvz
fi

if ! [ -d "$DIRECTORY/$FOLDER" ]; then
  cd $DIRECTORY
  mkdir ${FOLDER} ; cd ${FOLDER}
  ../configure CC=$(sh $DIRECTORY/build-scripts/gcc-fat.sh)
else
  cd $DIRECTORY/${FOLDER}
fi

make
make install

if [ -f "$DIRECTORY/$FOLDER/build/.libs/libSDL2main.a" ]; then
  cp $DIRECTORY/${FOLDER}/build/.libs/libSDL2main.a $DOME_DIR/lib
fi

cp $DIRECTORY/${FOLDER}/build/.libs/libSDL2.a $DOME_DIR/lib
cp $DIRECTORY/${FOLDER}/sdl2-config $DOME_DIR/lib/sdl2-config

cp -r $DIRECTORY/include $DOME_DIR/include/SDL2
cp -r $DIRECTORY/${FOLDER}/include/SDL_config.h  $DOME_DIR/include/SDL2/SDL_config.h
