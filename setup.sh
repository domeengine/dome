#!/bin/bash

rm -rf ./src/lib/wren
git clone https://github.com/munificent/wren.git ./src/lib/wren
cd src/lib/wren
make vm
cp lib/libwren.a ../libwren.a
