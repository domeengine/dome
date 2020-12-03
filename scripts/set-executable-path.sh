#!/bin/bash
TARGET_NAME=$1
if [[ "$OSTYPE" == "darwin"* ]]; then
install_name_tool -add_rpath \@executable_path/libSDL2-2.0.0.dylib $TARGET_NAME
install_name_tool -change /usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib \@executable_path/libSDL2-2.0.0.dylib $TARGET_NAME
install_name_tool -change /usr/local/lib/libSDL2-2.0.0.dylib \@executable_path/libSDL2-2.0.0.dylib $TARGET_NAME
fi
