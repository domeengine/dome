CC = cc
CFLAGS = -std=c99 -pedantic -Wall
OBJECTS = main.o test.o 
SOURCE  = src
BUILD  = build

all: game
game: $(SOURCE)/game.c
	$(CC) $(SOURCE)/game.c -o game -I$(SOURCE)/include -L$(SOURCE)/lib -lSDL2-2.0.0 -lwren

clean:
	    rm -f $(BUILD)/*.o game
