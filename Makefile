CC = cc
CFLAGS = -std=c99 -pedantic -Wall  -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-unused-value -Wno-incompatible-pointer-types-discards-qualifiers `sdl2-config --cflags` 
OBJECTS = main.o test.o 
SOURCE  = src
BUILD  = build

all: dome
src/lib/libwren.a: 
	./setup.sh
$(SOURCE)/engine/*.wren.inc: ${SOURCE}/util/embed.c ${SOURCE}/engine/*.wren
	cd src/util && ./generateEmbedModules.sh
dome: $(SOURCE)/*.c src/lib/libwren.a $(SOURCE)/engine/*.c ${SOURCE}/util/font.c ${SOURCE}/include $(SOURCE)/engine/*.wren.inc
	$(CC) $(CFLAGS) $(SOURCE)/main.c -o dome -lSDL2 -I$(SOURCE)/include -L$(SOURCE)/lib -lwren -lm

clean:
	    rm -rf $(BUILD)/*.o dome ${SOURCE}/lib/wren ${SOURCE}/lib/libwren.a ${SOURCE}/engine/*.inc
