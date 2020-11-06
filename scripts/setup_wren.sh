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
echo $config
unset config
MAKEFLAGS="--no-print-directory"
echo $config
make clean
make ${@:2} verbose=1 config=debug_$1 wren && cp ../../lib/libwren_d.a ../../../libwrend.a
make clean
make ${@:2} verbose=1 config=release_$1 wren && cp ../../lib/libwren.a ../../../libwren.a
