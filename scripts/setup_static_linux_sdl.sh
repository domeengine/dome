#!/bin/bash
VERSION=2.0.12
FOLDER=special
DOME_DIR=$PWD/src
DIRECTORY=$PWD/src/lib/SDL2-${VERSION}

if ! [ -d "$DIRECTORY" ]; then
  cd $DOME_DIR/lib
  wget -q -O- https://libsdl.org/release/SDL2-${VERSION}.tar.gz | tar -xvz
fi

if ! [ -d "$DIRECTORY/$FOLDER" ]; then
  cd $DIRECTORY
  mkdir ${FOLDER} ; cd ${FOLDER}
  # cmake -DSDL_SHARED=OFF -DSDL_TEST=OFF -DSDL_STATIC=ON -DJACK_SHARED=OFF -DPULSEAUDIO_SHARED=OFF -DALSA_SHARED=OFF -DSNDIO=OFF ..
  ../configure --disable-shared --enable-wayland-shared --enable-x11-shared
else
  cd $DIRECTORY/${FOLDER}
fi

make

if [ -f "$DIRECTORY/$FOLDER/build/.libs/libSDL2main.a" ]; then
  cp $DIRECTORY/${FOLDER}/build/.libs/libSDL2main.a $DOME_DIR/lib
fi

cp $DIRECTORY/${FOLDER}/build/.libs/libSDL2.a $DOME_DIR/lib
cp $DIRECTORY/${FOLDER}/sdl2-config $DOME_DIR/lib/sdl2-config
chmod +x $DOME_DIR/lib/sdl2-config

cp -r $DIRECTORY/include $DOME_DIR/include/SDL2
cp -r $DIRECTORY/${FOLDER}/include/SDL_config.h  $DOME_DIR/include/SDL2/SDL_config.h
