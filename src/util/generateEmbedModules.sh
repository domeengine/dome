#!/bin/bash
gcc embed.c -o embed -std=c99

declare -a arr=(
"dome" 
"ffi" 
"input" 
"graphics"
"io"
"audio"
"vector"
)

rm ../modules/modules.inc 2> /dev/null
touch ../modules/modules.inc

rm ../modules/modulemap.inc 2> /dev/null
touch ../modules/modulemap.c.inc

for i in "${arr[@]}"
do
  ./embed ../modules/${i}.wren ${i}Module ../modules/${i}.wren.inc
  echo "#include \"${i}.wren.inc\"" >> ../modules/modules.inc
  echo "ModuleMap_add(map, \"${i}\", ${i}Module);" >> ../modules/modulemap.c.inc
done
