BUILD_VALUE=$(shell git rev-parse --short HEAD)
CC = cc
CFLAGS = -std=c99 -pedantic -Wall  -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-unused-value -Wno-incompatible-pointer-types-discards-qualifiers `sdl2-config --cflags` 
IFLAGS = -I$(SOURCE)/include
LDFLAGS = -lSDL2 -lwren -lm -L$(SOURCE)/lib
SOURCE  = src
UTILS = $(SOURCE)/util
ENGINESRC = $(SOURCE)/engine
EXENAME = dome

all: $(EXENAME)

src/lib/libwren.a: 
	./setup.sh
	
$(ENGINESRC)/*.wren.inc: $(UTILS)/embed.c $(ENGINESRC)/*.wren
	cd $(UTILS) && ./generateEmbedModules.sh

$(EXENAME): $(SOURCE)/*.c src/lib/libwren.a $(ENGINESRC)/*.c $(UTILS)/font.c $(SOURCE)/include $(ENGINESRC)/*.wren.inc
	$(CC) $(CFLAGS) $(SOURCE)/main.c -o $(EXENAME) $(LDFLAGS) $(IFLAGS)

.PHONY: clean clean-all
clean-all:
	    rm -rf $(EXENAME) $(SOURCE)/lib/wren $(SOURCE)/lib/libwren.a $(ENGINESRC)/*.inc

clean:
	    rm -rf $(EXENAME) $(ENGINESRC)/*.inc

