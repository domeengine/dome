#!/bin/bash
gcc embed.c -o embed -std=c99

declare -a arr=(
"dome" 
"input" 
"graphics"
"io"
"audio"
"vector"
"image"
"math"
)
 
declare -a opts=(
"ffi" 
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

for i in "${opts[@]}"
do
  UPPER=`echo "${i}" | tr '[a-z]' '[A-Z]'`
  ./embed ../modules/${i}.wren ${i}Module ../modules/${i}.wren.inc
  echo "#if DOME_OPT_${UPPER}" >> ../modules/modules.inc
  echo "#include \"${i}.wren.inc\"" >> ../modules/modules.inc
  echo "#endif" >> ../modules/modules.inc
  echo "#if DOME_OPT_${UPPER}" >> ../modules/modulemap.c.inc
  echo "ModuleMap_add(map, \"${i}\", ${i}Module);" >> ../modules/modulemap.c.inc
  echo "#endif" >> ../modules/modulemap.c.inc
done
