CC = cc
CFLAGS = -std=c99 -pedantic -Wall -Wextra
OBJECTS = main.o test.o 
SOURCE  = src
BUILD  = build

all: dome
dome: $(SOURCE)/*.c
	$(CC) $(SOURCE)/main.c -o dome -I$(SOURCE)/include -L$(SOURCE)/lib -lSDL2-2.0.0 -lwren

clean:
	    rm -f $(BUILD)/*.o dome
