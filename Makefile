BUILD_VALUE=$(shell git rev-parse --short HEAD)
SOURCE  = src
CC = cc
CFLAGS = -std=c99 -pedantic -Wall  -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-unused-value `sdl2-config --cflags`
IFLAGS = -isystem $(SOURCE)/include
MODE ?= debug

SDLFLAGS=-lSDL2
LDFLAGS = -L$(SOURCE)/lib $(SDLFLAGS) -lm

ifeq ($(MODE), debug)
LDFLAGS += -lwrend
CFLAGS += -g -fsanitize=address
else
	LDFLAGS += -lwren
endif

UTILS = $(SOURCE)/util
ENGINESRC = $(SOURCE)/modules
EXENAME = dome

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

src/lib/libwren.a: 
	./setup.sh $(MODE)

src/include/wren.h: src/lib/libwren.a
	cp src/lib/wren/src/include/wren.h src/include/wren.h
	
$(ENGINESRC)/*.wren.inc: $(UTILS)/embed.c $(ENGINESRC)/*.wren
	cd $(UTILS) && ./generateEmbedModules.sh

$(EXENAME): $(SOURCE)/*.c src/lib/libwren.a $(ENGINESRC)/*.c $(UTILS)/font.c $(SOURCE)/include $(ENGINESRC)/*.wren.inc $(SOURCE)/include/wren.h
	$(CC) $(CFLAGS) $(SOURCE)/main.c -o $(EXENAME) $(LDFLAGS) $(IFLAGS)
ifneq (, $(findstring Darwin, $(SYS)))
	install_name_tool -change /usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib \@executable_path/libSDL2.dylib $(EXENAME)
endif

.PHONY: clean clean-all
clean-all:
	    rm -rf $(EXENAME) $(SOURCE)/lib/wren $(SOURCE)/lib/libwren.a $(ENGINESRC)/*.inc $(SOURCE)/include/wren.h $(SOURCE)/lib/libwrend.a

clean:
	    rm -rf $(EXENAME) $(ENGINESRC)/*.inc

