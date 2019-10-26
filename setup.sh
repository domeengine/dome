#!/bin/bash
MODE=${1:release}

git submodule update
cd src/lib/wren
make WREN_OPT_RANDOM=1 MODE=$MODE vm

if [ $MODE == "debug" ]; then
  cp lib/libwrend.a ../libwrend.a
else
  cp lib/libwren.a ../libwren.a
fi
