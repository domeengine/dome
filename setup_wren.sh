#!/bin/bash

git submodule update
cd src/lib/wren
make clean
make WREN_OPT_RANDOM=1 MODE=debug vm && cp lib/libwrend.a ../libwrend.a
make clean
make WREN_OPT_RANDOM=1 MODE=release vm && cp lib/libwren.a ../libwren.a
