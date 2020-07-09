#!/bin/bash

cd src/lib/wren/projects

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

make clean
make $@ config=debug_64bit wren && cp ../../lib/libwrend.a ../../../libwrend.a
make clean
make $@ MODE=release_64bit wren && cp ../../lib/libwren.a ../../../libwren.a
