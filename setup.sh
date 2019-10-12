#!/bin/bash

git submodule update
cd src/lib/wren
make WREN_OPT_RANDOM=1 vm
cp lib/libwren.a ../libwren.a
cp src/lib/wren/src/include/wren.h src/include/wren.h
