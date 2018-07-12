#!/bin/bash

cd src/lib/wren
make vm
cp lib/libwren.a ../libwren.a
