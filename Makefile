CC = cc
EXENAME = dome
MODE_FILE=.mode

SOURCE  = src
UTILS = $(SOURCE)/util
LIBS = $(SOURCE)/lib
INCLUDES = $(SOURCE)/include
MODULES = $(SOURCE)/modules
EXAMPLES = examples

MODE ?= $(shell cat $(MODE_FILE) 2>/dev/null || echo release)
SDL_CONFIG ?= src/lib/SDL2/bin/sdl2-config
BUILD_VALUE=$(shell git rev-parse --short HEAD)
SYS=$(shell uname -s)


DOME_OPTS = -DHASH="\"$(BUILD_VALUE)\""
CFLAGS = $(DOME_OPTS) -std=c99 -pedantic -Wall  -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-unused-value `which sdl2-config 1>/dev/null && sdl2-config --cflags`
IFLAGS = -isystem $(INCLUDES)
ifdef STATIC
$(warning "STATIC")
SDLFLAGS = `$(SDL_CONFIG) --static-libs`
# SDLFLAGS = `which sdl2-config 1>/dev/null && sdl2-config --static-libs || echo "-lSDL2 -lSDL2main"`
IFLAGS := -I$(LIBS)/SDL2-2.0.2/include $(IFLAGS)
else
SDLFLAGS = `which sdl2-config 1>/dev/null && sdl2-config --libs`
endif
LDFLAGS = -L$(LIBS) $(SDLFLAGS) -lm

## Optional DOME Module Switches
DOME_OPT_FFI=0
ifeq ($(DOME_OPT_FFI),1)
  DOME_OPTS += -D DOME_OPT_FFI=1
  LDFLAGS  += -lffi
  FFI_DEPS = $(LIBS)/libffi $(LIBS)/libffi.a $(INCLUDES)/ffi.h
endif


## Handle Release/Debug build things
ifeq ($(MODE), debug)
	LDFLAGS += -lwrend
	CFLAGS += -g -O0
ifneq ($(EXTRA), valgrind)
	CFLAGS += -fsanitize=address
endif

  DOME_OPTS += -DDEBUG=1
$(shell echo $(MODE) > $(MODE_FILE))
else
	LDFLAGS += -lwren
	CFLAGS += -O3
$(shell echo $(MODE) > $(MODE_FILE))
endif


## Handle OS Specific commands
ifneq (, $(findstring Darwin, $(SYS)))
  FRAMEWORK ?= $(shell which sdl2-config && echo unix || echo framework)
  CFLAGS += -Wno-incompatible-pointer-types-discards-qualifiers

  ifdef MIN_MAC_VERSION
    CFLAGS += -mmacosx-version-min=$(MIN_MAC_VERSION)
  endif

  ifeq ($(FRAMEWORK), framework)
    CFLAGS +=  -I /Library/Frameworks/SDL2.framework/Headers -framework SDL2
  endif
endif

ifneq (, $(findstring MINGW, $(SYS)))
  WINDOW_MODE ?= windows
	SDLFLAGS := -m$(WINDOW_MODE) $(SDLFLAGS)
  CFLAGS += -Wno-discarded-qualifiers -Wno-clobbered
  ifdef ICON_OBJECT_FILE
    CFLAGS += $(ICON_OBJECT_FILE)
  endif
endif

ifneq (, $(findstring Linux, $(SYS)))
  CFLAGS += -Wno-discarded-qualifiers -Wno-clobbered
endif



.PHONY: all clean reset cloc
all: $(EXENAME)

$(LIBS)/libffi/autogen.sh:
	git submodule update --init -- $(LIBS)/libffi
$(LIBS)/libffi: $(LIBS)/libffi/autogen.sh

$(LIBS)/wren/Makefile: 
	git submodule update --init -- $(LIBS)/wren
$(LIBS)/wren: $(LIBS)/wren/Makefile
	
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

$(EXENAME): $(SOURCE)/*.c $(MODULES)/*.c $(UTILS)/font.c $(INCLUDES) $(MODULES)/*.inc $(INCLUDES)/wren.h $(LIBS)/libwren.a $(FFI_DEPS)
	$(CC) $(CFLAGS) $(SOURCE)/main.c -o $(EXENAME) $(LDFLAGS) $(IFLAGS)
	$(warning $(MODE))
ifneq (, $(findstring Darwin, $(SYS)))
ifneq ($(FRAMEWORK), framework)
	install_name_tool -change /usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib \@executable_path/libSDL2-2.0.0.dylib $(EXENAME)
	install_name_tool -change /usr/local/lib/libSDL2-2.0.0.dylib \@executable_path/libSDL2-2.0.0.dylib $(EXENAME)
else
	install_name_tool -add_rpath \@executable_path/libSDL2-2.0.0.dylib $(EXENAME)
endif
endif

# Used for the example game FFI test
libadd.so: $(EXAMPLES)/ffi/add.c
	$(CC) -O -fno-common -c $(EXAMPLES)/ffi/add.c $(IFLAGS) -o $(EXAMPLES)/ffi/add.o -g
	$(CC) -flat_namespace -bundle -undefined suppress -o $(EXAMPLES)/ffi/libadd.so $(EXAMPLES)/ffi/add.o
	rm $(EXAMPLES)/ffi/add.o

reset:
	git submodule foreach --recursive git clean -xfd
	rm -rf .mode $(EXENAME) $(LIBS)/libwren.a $(MODULES)/*.inc $(INCLUDES)/wren.h $(LIBS)/libwrend.a $(LIBS)/libffi.a $(INCLUDES)/ffi.h $(INCLUDES)/ffitarget.h

clean:
	rm -rf $(EXENAME) $(MODULES)/*.inc

# Counts the number of lines used, for vanity
cloc:
	cloc --by-file --force-lang="java",wren --fullpath --not-match-d "util|include|lib" -not-match-f ".inc" src

