#!/bin/bash

cd src/lib/wren
make clean
make $@ MODE=debug vm && cp lib/libwrend.a ../libwrend.a
make clean
make $@ MODE=release vm && cp lib/libwren.a ../libwren.a
