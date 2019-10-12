#!/bin/bash

git submodule update
cd src/lib/wren
make WREN_OPT_RANDOM=1 MODE=debug vm
cp lib/libwrend.a ../libwren.a
cp src/include/wren.h ../../include/wren.h
