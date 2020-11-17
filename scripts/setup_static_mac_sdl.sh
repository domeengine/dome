#!/bin/bash
source ./scripts/vars.sh

if ! [ -d "$DIRECTORY" ]; then
  cd $LIB_DIR
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
# make install

if [ -f "$DIRECTORY/$FOLDER/build/.libs/libSDL2main.a" ]; then
  cp $DIRECTORY/${FOLDER}/build/.libs/libSDL2main.a $LIB_DIR
fi

cp $DIRECTORY/${FOLDER}/build/.libs/libSDL2.a $LIB_DIR
cp $DIRECTORY/${FOLDER}/build/.libs/libSDL2-2.0.0.dylib $LIB_DIR
cp $DIRECTORY/${FOLDER}/build/.libs/libSDL2.dylib $LIB_DIR
cp $DIRECTORY/${FOLDER}/sdl2-config $LIB_DIR/sdl2-config

cp -r $DIRECTORY/include $INCLUDE_DIR/SDL2
cp -r $DIRECTORY/${FOLDER}/include/SDL_config.h  $INCLUDE_DIR/SDL2/SDL_config.h
