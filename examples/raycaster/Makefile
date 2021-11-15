NAME = raycaster
CFLAGS = -O0 -g
.PHONY: all

all: ${NAME}.dylib

renderer.c.inc: renderer.wren
	../../dome embed renderer.wren rendererModuleSource renderer.c.inc

${NAME}.dylib: ${NAME}.c domath.c sbuf.c sort.c renderer.c.inc
	gcc ${CFLAGS} -dynamiclib -o ${NAME}.dylib -I../../include ${NAME}.c -undefined dynamic_lookup

${NAME}.so: ${NAME}.c renderer.c.inc
	gcc ${CFLAGS} -shared -std=c11 -o ${NAME}.so -fPIC  -I../../include ${NAME}.c

${NAME}.dll: ${NAME}.c renderer.c.inc
	gcc ${CFLAGS} -shared -std=gnu11 -shared -fPIC  -I../../include ${NAME}.c -Wl,--unresolved-symbols=ignore-in-object-files -o ${NAME}.dll
