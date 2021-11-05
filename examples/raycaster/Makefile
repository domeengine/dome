NAME = raycaster
.PHONY: ${NAME}.dylib
${NAME}.dylib: ${NAME}.c
	gcc -dynamiclib -o ${NAME}.dylib -I../../include ${NAME}.c -undefined dynamic_lookup

${NAME}.so: ${NAME}.c
	gcc -O3 -std=c11 -shared -o ${NAME}.so -fPIC  -I../../include ${NAME}.c

${NAME}.dll: ${NAME}.c
	gcc -O3 -std=gnu11 -shared -fPIC  -I../../include ${NAME}.c -Wl,--unresolved-symbols=ignore-in-object-files -o ${NAME}.dll
