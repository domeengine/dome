#!/bin/bash

cd src/lib/libffi
./autogen.sh
./configure --prefix=$PWD
make && make install
cp lib/libffi.a ../libffi.a
cp include/ffi.h ../../include/ffi.h
cp include/ffitarget.h ../../include/ffitarget.h


