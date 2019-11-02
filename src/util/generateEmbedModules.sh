#!/bin/bash
gcc embed.c -o embed -std=c99

declare -a arr=("dome" 
"input" 
"graphics"
"io"
"audio"
"vector")

rm ../modules/modules.inc 2> /dev/null
touch ../modules/modules.inc

rm ../modules/modulemap.inc 2> /dev/null
touch ../modules/modulemap.c.inc

for i in "${arr[@]}"
do
  ./embed ../modules/${i}.wren ${i}Module ../modules/${i}.wren.inc
  echo "#include \"${i}.wren.inc\"" >> ../modules/modules.inc
  # or do whatever with individual element of the array
  echo "ModuleMap_add(map, \"${i}\", ${i}Module);" >> ../modules/modulemap.c.inc
done

# ./embed ../modules/dome.wren domeModule ../modules/dome.wren.inc
# ./embed ../modules/input.wren inputModule ../modules/input.wren.inc
# ./embed ../modules/graphics.wren graphicsModule ../modules/graphics.wren.inc
# ./embed ../modules/io.wren ioModule ../modules/io.wren.inc
# ./embed ../modules/audio.wren audioModule ../modules/audio.wren.inc
# ./embed ../modules/point.wren pointModule ../modules/point.wren.inc
