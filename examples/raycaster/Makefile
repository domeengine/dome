NAME = raycaster
CFLAGS = -O0 -g
.PHONY: all clean

all: ${NAME}.dylib
SOURCES	= $(wildcard *.c) $(wildcard *.h)
WREN_SOURCES = renderer.wren.inc

renderer.wren.inc: renderer.wren
	../../dome embed renderer.wren rendererModuleSource renderer.wren.inc

${NAME}.dylib: ${SOURCES} ${WREN_SOURCES}
	${CC} ${CFLAGS} -dynamiclib -o ${NAME}.dylib -I../../include plugin.c -undefined dynamic_lookup -lm

${NAME}.so: ${SOURCES} ${WREN_SOURCES}
	${CC} ${CFLAGS} -shared -std=c11 -o ${NAME}.so -fPIC  -I../../include plugin.c -lm

${NAME}.dll: ${SOURCES} ${WREN_SOURCES}
	${CC} ${CFLAGS} -shared -std=gnu11 -shared -fPIC  -I../../include plugin.c -Wl,--unresolved-symbols=ignore-in-object-files -o ${NAME}.dll -lm

clean:
	rm -f ${NAME}.dylib ${NAME}.so ${NAME}.dll *.wren.inc *.o
