CC = cc
CFLAGS = -std=c99 -pedantic -Wall -Wextra -Wno-unused-parameter -Wno-unused-function 
OBJECTS = main.o test.o 
SOURCE  = src
BUILD  = build

all: dome
src/lib/libwren.a: 
	./setup.sh
dome: $(SOURCE)/*.c src/lib/libwren.a $(SOURCE)/engine/*.c ${SOURCE}/util/font.c ${SOURCE}/include
	$(CC) $(CFLAGS) $(SOURCE)/main.c -o dome -I$(SOURCE)/include -L$(SOURCE)/lib -lSDL2-2.0.0 -lwren

clean:
	    rm -rf $(BUILD)/*.o dome ${SOURCE}/lib/wren ${SOURCE}/lib/libwren.a
