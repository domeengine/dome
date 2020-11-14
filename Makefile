
# Paths
SOURCE=src
LIBS=lib
UTILS = $(SOURCE)/util
INCLUDES=$(SOURCE)/include
MODULES=$(SOURCE)/modules
SCRIPTS=scripts


# Build flags
# Each must have distinct values for dimension

# MODE: release or debug
MODE ?= release

# Determine the system
# ARCH = 64bit or 32bit
UNAME_S = $(shell uname -s)
UNAME_P = $(shell uname -p)
ifeq ($(UNAME_S), Darwin)
SYSTEM ?= macosx
ARCH ?= 64bit
FRAMEWORK ?= $(shell which sdl2-config 1>/dev/null && echo "" || echo "framework")
else ifeq ($(UNAME_S), Linux)
SYSTEM ?= linux
ARCH ?= 64bit
else
SYSTEM ?= windows
ifneq (,$(findstring 32,$(UNAME_S)))
	ARCH ?= 32bit
else
	ARCH ?= 64bit
endif
endif

# 0 or 1
STATIC ?= 0
TAGS = $(ARCH) $(SYSTEM) $(MODE) $(FRAMEWORK)

ifeq ($(STATIC), 1)
TAGS += static
else 
TAGS += shared
endif

ifndef verbose
  SILENT = @
endif

# Compute Variables based on build flags

ifneq ($(and $(filter windows,$(TAGS)),$(filter 64bit,$(TAGS))),)
TARGET_NAME ?= dome-x64
ICON_OBJECT_FILE ?= assets/icon64.res
else ifneq ($(and $(filter windows,$(TAGS)),$(filter 32bit,$(TAGS))),)
TARGET_NAME ?= dome-x32
ICON_OBJECT_FILE ?= assets/icon32.res
else
TARGET_NAME ?= dome
endif

BUILD_VALUE=$(shell git rev-parse --short HEAD)
DOME_OPTS = -DDOME_HASH=\"$(BUILD_VALUE)\"
ifdef DOME_OPT_VERSION
  DOME_OPTS += -DDOME_VERSION=\"$(DOME_OPT_VERSION)\"
else
  DOME_OPTS += -DDOME_VERSION=\"$(shell git describe --tags)\"
endif

SDL_CONFIG ?= $(shell which sdl2-config 1>/dev/null && echo "sdl2-config" || echo "$(LIBS)/sdl2-config")

# Compiler configurations

WARNING_FLAGS = -Wall -Wno-unused-parameter -Wno-unused-function -Wno-unused-value

ifneq ($(filter macosx,$(TAGS)),)
WARNING_FLAGS += -Wno-incompatible-pointer-types-discards-qualifiers
else ifneq ($(filter windows,$(TAGS)),)
WARNING_FLAGS += -Wno-discarded-qualifiers -Wno-clobbered
else ifneq ($(filter linux,$(TAGS)),)
	WARNING_FLAGS += -Wno-clobbered
endif


CFLAGS = $(DOME_OPTS) -std=c99 -pedantic $(WARNING_FLAGS)
ifneq ($(filter macosx,$(TAGS)),)
CFLAGS += -mmacosx-version-min=10.12

ifneq ($(and $(filter framework,$(TAGS)), $(filter shared, $(TAGS))),)
CFLAGS := -framework SDL2 $(CFLAGS) -F/Library/Frameworks -framework SDL2
endif

else ifneq ($(filter windows,$(TAGS)),)

ifdef ICON_OBJECT_FILE
	CFLAGS += $(ICON_OBJECT_FILE)
endif

endif

ifneq ($(filter release,$(TAGS)),)
CFLAGS += -O3
else ifneq ($(filter debug,$(TAGS)),)
CFLAGS += -g -O0
endif

# Include Configuration
IFLAGS = -isystem $(INCLUDES)
IFLAGS += $(shell $(SDL_CONFIG) --cflags)
ifneq ($(filter static,$(TAGS)),)
IFLAGS := -I$(INCLUDES)/SDL2 $(IFLAGS)
else ifneq ($(filter framework,$(TAGS)),)
IFLAGS += -I /Library/Frameworks/SDL2.framework/Headers
endif


# Compute Link Settings
DEPS = -lm

ifneq ($(filter static,$(TAGS)),)
SDLFLAGS=$(shell $(SDL_CONFIG) --static-libs)
else
SDLFLAGS=$(shell $(SDL_CONFIG) --libs)
endif

ifneq ($(filter release,$(TAGS)),)
DEPS += -lwren
else ifneq ($(filter debug,$(TAGS)),)
DEPS += -lwrend
endif
ifneq ($(and $(filter windows,$(TAGS)),$(filter static,$(TAGS))),)
WINDOW_MODE ?= windows
WINDOW_MODE_FLAG = -m$(WINDOW_MODE)
STATIC_FLAG += -static
endif

LDFLAGS = -L$(LIBS) $(WINDOW_MODE_FLAG) $(SDLFLAGS) $(STATIC_FLAG) $(DEPS)




# Build Rules


PROJECTS := dome.bin wren modules static
.PHONY: all clean reset cloc $(PROJECTS)

all: $(PROJECTS)

WREN_LIB ?= $(LIBS)/libwren.a
WREN_PARAMS ?= $(ARCH) WREN_OPT_RANDOM=1 WREN_OPT_META=1   
$(LIBS)/wren/lib/libwren.a:
	@echo "==== Cloning Wren ===="
	git submodule update --init -- $(LIBS)/wren
$(LIBS)/wren: $(LIBS)/wren/lib/libwren.a
$(WREN_LIB): $(LIBS)/wren
	@echo "==== Building Wren ===="
	./scripts/setup_wren.sh $(WREN_PARAMS)
	cp $(LIBS)/wren/src/include/wren.h $(INCLUDES)/wren.h
wren: $(WREN_LIB)

$(MODULES)/*.inc: $(UTILS)/embed.c $(MODULES)/*.wren
	@echo "==== Building DOME modules  ===="
	./scripts/generateEmbedModules.sh
modules: $(MODULES)/*.inc

$(TARGET_NAME): wren modules $(SOURCE)/*.c $(MODULES)/*.c $(INCLUDES)
	@echo "==== Building DOME ($(TAGS)) ===="
	$(CC) $(CFLAGS) $(SOURCE)/main.c -o $(TARGET_NAME) $(LDFLAGS) $(IFLAGS)
ifneq ($(and $(filter macosx,$(TAGS)),$(filter framework,$(TAGS))),)
	install_name_tool -add_rpath \@executable_path/libSDL2-2.0.0.dylib $(TARGET_NAME)
else
	install_name_tool -change /usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib \@executable_path/libSDL2-2.0.0.dylib $(TARGET_NAME)
	install_name_tool -change /usr/local/lib/libSDL2-2.0.0.dylib \@executable_path/libSDL2-2.0.0.dylib $(TARGET_NAME)
endif
	@echo "Build DOME as $(TARGET_NAME)"



dome.bin: $(TARGET_NAME)


clean: 
	rm -rf $(TARGET_NAME) $(MODULES)/*.inc
reset:
	git submodule foreach --recursive git clean -xfd
	rm -rf $(LIBS)/libwren.a 
	rm -rf $(LIBS)/libwrend.a
	rm -rf $(INCLUDES)/wren.h 
