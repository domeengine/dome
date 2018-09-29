BUILD_VALUE=$(shell git rev-parse --short HEAD)
CFLAGS = -std=c99 -pedantic -Wall  -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-unused-value
CFLAGS += `sdl2-config --cflags`
IFLAGS = -I$(SOURCE)/include


SDLFLAGS=-lSDL2
LDFLAGS = -L$(SOURCE)/lib $(SDLFLAGS) -lwren -lm
SOURCE  = src
UTILS = $(SOURCE)/util
ENGINESRC = $(SOURCE)/engine
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
	git submodule update
	cd src/lib/wren && make WREN_OPT_RANDOM=1 static
	cp src/lib/wren/lib/libwren.a src/lib/libwren.a

	
$(ENGINESRC)/*.wren.inc: $(UTILS)/embed.c $(ENGINESRC)/*.wren
	cd $(UTILS) && ./generateEmbedModules.sh

$(EXENAME): $(SOURCE)/*.c src/lib/libwren.a $(ENGINESRC)/*.c $(UTILS)/font.c $(SOURCE)/include $(ENGINESRC)/*.wren.inc
	$(CC) $(CFLAGS) $(SOURCE)/main.c -o $(EXENAME) $(LDFLAGS) $(IFLAGS)

ifneq (, $(findstring Darwin, $(SYS)))
#	install_name_tool -change /usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib \@executable_path/libSDL2.dylib $(EXENAME)
endif

.PHONY: clean clean-all
clean-all:
	    rm -rf $(EXENAME) $(SOURCE)/lib/wren $(SOURCE)/lib/libwren.a $(ENGINESRC)/*.inc

clean:
	    rm -rf $(EXENAME) $(ENGINESRC)/*.inc

