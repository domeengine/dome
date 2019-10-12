#!/bin/bash

git submodule update
cd src/lib/wren
make WREN_OPT_RANDOM=1 vm
cp lib/libwren.a ../libwren.a
cp src/include/wren.h ../../include/wren.h
