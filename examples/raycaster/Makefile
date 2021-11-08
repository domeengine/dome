NAME = raycaster
CFLAGS = -O3 -g
.PHONY: ${NAME}.dylib
${NAME}.dylib: ${NAME}.c
	gcc ${CFLAGS} -dynamiclib -o ${NAME}.dylib -I../../include ${NAME}.c -undefined dynamic_lookup

${NAME}.so: ${NAME}.c
	gcc ${CFLAGS} -shared -std=c11 -o ${NAME}.so -fPIC  -I../../include ${NAME}.c

${NAME}.dll: ${NAME}.c
	gcc ${CFLAGS} -shared -std=gnu11 -shared -fPIC  -I../../include ${NAME}.c -Wl,--unresolved-symbols=ignore-in-object-files -o ${NAME}.dll
