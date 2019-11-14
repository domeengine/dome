MODE_FILE=.mode
MODE ?= $(shell cat $(MODE_FILE) 2>/dev/null || echo release)

BUILD_VALUE=$(shell git rev-parse --short HEAD)
SOURCE  = src
CC = cc
CFLAGS = -std=c99 -pedantic -Wall  -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-unused-value `sdl2-config --cflags`
IFLAGS = -isystem $(SOURCE)/include
SDLFLAGS=-lSDL2
LDFLAGS = -L$(SOURCE)/lib $(SDLFLAGS) -lm -lffi

UTILS = $(SOURCE)/util
ENGINESRC = $(SOURCE)/modules

EXENAME = dome

ifeq ($(MODE), debug)
	LDFLAGS += -lwrend
	CFLAGS += -g -fsanitize=address -O0
  $(shell echo $(MODE) > .mode)
else
	LDFLAGS += -lwren
	CFLAGS += -O3
  $(shell echo $(MODE) > .mode)
endif

SYS=$(shell uname -s)

ifneq (, $(findstring Darwin, $(SYS)))
	CFLAGS += -Wno-incompatible-pointer-types-discards-qualifiers
endif

ifneq (, $(findstring MSYS, $(SYS)))
	CFLAGS += -Wno-discarded-qualifiers
	ifdef ICON_OBJECT_FILE
	CFLAGS += $(ICON_OBJECT_FILE)
endif
SDLFLAGS := -lSDL2main -mwindows $(SDLFLAGS)
endif

ifneq (, $(findstring Linux, $(SYS)))
	CFLAGS += -Wno-discarded-qualifiers
endif


all: $(EXENAME)

$(SOURCE)/lib/wren: 
	./setup.sh

$(SOURCE)/include/wren.h: $(SOURCE)/lib/wren
	cp src/lib/wren/src/include/wren.h src/include/wren.h

$(ENGINESRC)/*.inc: $(UTILS)/embed.c $(ENGINESRC)/*.wren
	cd $(UTILS) && ./generateEmbedModules.sh

$(EXENAME): $(SOURCE)/*.c $(SOURCE)/lib/wren $(ENGINESRC)/*.c $(UTILS)/font.c $(SOURCE)/include $(ENGINESRC)/*.inc $(SOURCE)/include/wren.h
	$(CC) $(CFLAGS) $(SOURCE)/main.c -o $(EXENAME) $(LDFLAGS) $(IFLAGS)
	$(warning $(MODE))
ifneq (, $(findstring Darwin, $(SYS)))
	install_name_tool -change /usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib \@executable_path/libSDL2.dylib $(EXENAME)
endif

.PHONY: clean clean-all cloc
clean-all:
	rm -rf $(EXENAME) $(SOURCE)/lib/wren $(SOURCE)/lib/libwren.a $(ENGINESRC)/*.inc $(SOURCE)/include/wren.h $(SOURCE)/lib/libwrend.a

clean:
	rm -rf $(EXENAME) $(ENGINESRC)/*.inc

cloc:
	cloc --by-file --force-lang="java",wren --fullpath --not-match-d "util|include|lib" -not-match-f ".inc" src

libadd.so: test/add.c
	$(CC) -O -fno-common -c test/add.c $(IFLAGS) -o test/add.o -g
	$(CC) -flat_namespace -bundle -undefined suppress -o libadd.so test/add.o
	rm test/add.o
