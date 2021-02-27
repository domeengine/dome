#!/bin/bash
source ./scripts/vars.sh
git submodule update --init -- $DIRECTORY

if ! [ -d "$DIRECTORY/$FOLDER" ]; then
  cd $DIRECTORY
  mkdir ${FOLDER} ; cd ${FOLDER}
  # cmake -DSDL_SHARED=OFF -DSDL_TEST=OFF -DSDL_STATIC=ON -DJACK_SHARED=OFF -DPULSEAUDIO_SHARED=OFF -DALSA_SHARED=OFF -DSNDIO=OFF ..
  ../configure --disable-shared --enable-wayland-shared --enable-x11-shared
else
  cd $DIRECTORY/${FOLDER}
fi

make

# These two are essential
cp $DIRECTORY/${FOLDER}/build/.libs/libSDL2.a $LIB_DIR
cp $DIRECTORY/${FOLDER}/sdl2-config $LIB_DIR/sdl2-config

if [ -f "$DIRECTORY/$FOLDER/build/.libs/libSDL2main.a" ]; then
  cp $DIRECTORY/${FOLDER}/build/.libs/libSDL2main.a $LIB_DIR
fi
if [ -f "$DIRECTORY/$FOLDER/build/.libs/libSDL2main.so" ]; then
  cp $DIRECTORY/${FOLDER}/build/.libs/libSDL2main.so $LIB_DIR
fi

if [ -f "$DIRECTORY/$FOLDER/build/.libs/libSDL2.so" ]; then
cp $DIRECTORY/${FOLDER}/build/.libs/libSDL2.so $LIB_DIR
fi

chmod +x $LIB_DIR/sdl2-config

cp -r $DIRECTORY/include $INCLUDE_DIR/SDL2
cp -r $DIRECTORY/${FOLDER}/include/SDL_config.h  $INCLUDE_DIR/SDL2/SDL_config.h
