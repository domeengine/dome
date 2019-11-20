MODE_FILE=.mode
MODE ?= $(shell cat $(MODE_FILE) 2>/dev/null || echo release)

SOURCE  = src
UTILS = $(SOURCE)/util
LIBS = $(SOURCE)/lib
INCLUDES = $(SOURCE)/include
MODULES = $(SOURCE)/modules

BUILD_VALUE=$(shell git rev-parse --short HEAD)
CC = cc
CFLAGS = -std=c99 -pedantic -Wall  -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-unused-value `sdl2-config --cflags`
IFLAGS = -isystem $(INCLUDES)
SDLFLAGS=-lSDL2
LDFLAGS = -L$(LIBS) $(SDLFLAGS) -lm -lffi

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

$(LIBS)/libffi: 
$(LIBS)/wren: 
	git submodule init
	git submodule update
	
$(LIBS)/libffi.a: $(LIBS)/libffi
	./setup_ffi.sh

$(LIBS)/libwren.a: $(LIBS)/wren
	./setup_wren.sh

$(INCLUDES)/ffi.h: $(LIBS)/libffi.a
$(INCLUDES)/ffitarget.h: $(LIBS)/libffi.a
	
$(INCLUDES)/wren.h: $(LIBS)/libwren.a
	cp src/lib/wren/src/include/wren.h src/include/wren.h

$(MODULES)/*.inc: $(UTILS)/embed.c $(MODULES)/*.wren
	cd $(UTILS) && ./generateEmbedModules.sh

$(EXENAME): $(SOURCE)/*.c $(MODULES)/*.c $(UTILS)/font.c $(INCLUDES) $(MODULES)/*.inc $(INCLUDES)/wren.h $(INCLUDES)/ffi.h $(LIBS)/libwren.a $(LIBS)/libffi.a
	$(CC) $(CFLAGS) $(SOURCE)/main.c -o $(EXENAME) $(LDFLAGS) $(IFLAGS)
	$(warning $(MODE))
ifneq (, $(findstring Darwin, $(SYS)))
	install_name_tool -change /usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib \@executable_path/libSDL2.dylib $(EXENAME)
endif

.PHONY: clean clean-all cloc
clean-all:
	rm -rf $(EXENAME) $(LIBS)/wren $(LIBS)/libwren.a $(MODULES)/*.inc $(INCLUDES)/wren.h $(LIBS)/libwrend.a $(LIBS)/libffi  $(LIBS)/libffi.a $(INCLUDES)/ffi.h $(INCLUDES)/ffitarget..a

clean:
	rm -rf $(EXENAME) $(MODULES)/*.inc

cloc:
	cloc --by-file --force-lang="java",wren --fullpath --not-match-d "util|include|lib" -not-match-f ".inc" src

libadd.so: test/add.c
	$(CC) -O -fno-common -c test/add.c $(IFLAGS) -o test/add.o -g
	$(CC) -flat_namespace -bundle -undefined suppress -o libadd.so test/add.o
	rm test/add.o
