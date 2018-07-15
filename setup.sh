#!/bin/bash

cd src/lib/wren
make WREN_OPT_RANDOM=1 vm
cp lib/libwren.a ../libwren.a
