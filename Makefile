# Paths
SOURCE=src
LIBS=lib
OBJS=obj
INCLUDES=include
SOURCE_FILES = $(shell find src -type f)
TOOLS = $(SOURCE)/tools
MODULES=$(SOURCE)/modules
SCRIPTS=scripts
WREN_LIB ?= $(OBJS)/libwren.o
WREN_PARAMS ?= -DWREN_OPT_RANDOM=0 -DWREN_OPT_META=1


# Build flags
# Each must have distinct values for dimension

# MODE: release or debug
MODE ?= release

# Determine the system
# ARCH = 64bit or 32bit
UNAME_S = $(shell uname -s)
UNAME_P = $(shell uname -p)
UNAME_M = $(shell uname -m)
ifeq ($(UNAME_S), Darwin)
SYSTEM ?= macosx
ARCH ?= 64bit
FRAMEWORK ?= $(shell which sdl2-config 1>/dev/null && echo "" || echo "framework")
else ifeq ($(UNAME_S), Linux)
SYSTEM ?= linux
	ifeq ($(UNAME_M), aarch64)
		ARCH ?= arm64
	else
		ARCH ?= 64bit
	endif
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
TAGS = $(ARCH) $(SYSTEM) $(MODE) $(FRAMEWORK) $(SYMBOLS)

ifneq ($(filter debug,$(TAGS)),)
TAGS += symbols
WREN_PARAMS += -DDEBUG=1
endif

OBJS := $(OBJS)/$(ARCH)

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
ifneq ($(filter windows,$(TAGS)),)
  DOME_OPTS += -D__USE_MINGW_ANSI_STDIO=1
endif

SDL_CONFIG ?= $(shell which "$(LIBS)/sdl2-config" 1>/dev/null && echo "$(LIBS)/sdl2-config" || (which "sdl2-config" 1>/dev/null && echo "sdl2-config" || echo ""))

# Compiler configurations

WARNING_FLAGS = -Wall -Wno-unused-parameter -Wno-unused-function -Wno-unused-value

ifneq ($(filter macosx,$(TAGS)),)
WARNING_FLAGS += -Wno-incompatible-pointer-types-discards-qualifiers
else ifneq ($(filter windows,$(TAGS)),)
WARNING_FLAGS += -Wno-discarded-qualifiers -Wno-clobbered
else ifneq ($(filter linux,$(TAGS)),)
	WARNING_FLAGS += -Wno-clobbered -Wno-maybe-uninitialized -Wno-attributes
endif


CFLAGS = $(DOME_OPTS) -std=c99 -pedantic $(WARNING_FLAGS) -fvisibility=hidden
ifneq ($(filter linux,$(TAGS)),)
CFLAGS += -D_XOPEN_SOURCE=500
else ifneq ($(filter macosx,$(TAGS)),)
CFLAGS += -mmacosx-version-min=10.12
CFLAGS += -D_DARWIN_C_SOURCE
endif

ifneq ($(filter release,$(TAGS)),)
CFLAGS += -O2
else ifneq ($(filter debug,$(TAGS)),)
CFLAGS += -O0
ifneq ($(filter macosx,$(TAGS)),)
CFLAGS += -fsanitize=address
FFLAGS += -fsanitize=address
endif
endif

ifneq ($(filter symbols,$(TAGS)),)
CFLAGS += -g 
endif

# Include Configuration
IFLAGS = -isystem $(INCLUDES)
ifneq (,$(findstring sdl2-config, $(SDL_CONFIG)))
IFLAGS += $(shell $(SDL_CONFIG) --cflags)
endif
ifneq ($(filter static,$(TAGS)),)
IFLAGS := -I$(INCLUDES)/SDL2 $(IFLAGS)
else ifneq ($(filter framework,$(TAGS)),)
IFLAGS += -I/Library/Frameworks/SDL2.framework/Headers
endif


# Compute Link Settings
DEPS = -lm

ifneq (,$(findstring sdl2-config, $(SDL_CONFIG)))
ifneq ($(filter static,$(TAGS)),)
SDLFLAGS=$(shell $(SDL_CONFIG) --static-libs)
else
SDLFLAGS=$(shell $(SDL_CONFIG) --libs)
endif
endif

ifneq ($(and $(filter windows,$(TAGS)),$(filter static,$(TAGS))),)
WINDOW_MODE ?= windows
WINDOW_MODE_FLAG = -m$(WINDOW_MODE)
STATIC_FLAG += -static

else ifneq ($(and $(filter macosx,$(TAGS)), $(filter framework,$(TAGS)), $(filter shared,$(TAGS))),)
FFLAGS += -F/Library/Frameworks -framework SDL2
endif

LDFLAGS = -L$(LIBS) $(WINDOW_MODE_FLAG) $(SDLFLAGS) $(STATIC_FLAG)
ifneq ($(and $(filter linux,$(TAGS)), $(filter 64bit, $(TAGS))),)
	COMPAT_DEP = $(OBJS)/glibc_compat.o
	LDFLAGS += -Wl,--wrap=log,--wrap=log2,--wrap=exp,--wrap=pow,--wrap=expf,--wrap=powf,--wrap=logf
endif
LDFLAGS += $(DEPS)




# Build Rules
PROJECTS := dome.bin modules
.PHONY: all clean reset cloc $(PROJECTS)

all: $(PROJECTS)

$(TOOLS)/embed: $(TOOLS)/embed-standalone.c $(TOOLS)/embedlib.c
	@echo "==== Building standalone embed tool  ===="
	$(CC) -o $(TOOLS)/embed $(CFLAGS) $(TOOLS)/embed-standalone.c $(WINDOW_MODE_FLAG)

$(MODULES)/*.inc: $(TOOLS)/embed $(MODULES)/*.wren
	@echo "==== Building DOME modules  ===="
	./scripts/generateEmbedModules.sh

modules: $(MODULES)/*.inc

$(OBJS)/libwren.o: $(INCLUDES)/wren.c
	@mkdir -p $(OBJS)
	@echo "==== Building wren module ===="
	$(CC) -c $(INCLUDES)/wren.c -o $(OBJS)/libwren.o $(WREN_PARAMS) $(IFLAGS)

$(OBJS)/glibc_compat.o: $(INCLUDES)/glibc_compat.c
	@mkdir -p $(OBJS)
	@echo "==== Building glibc_compat module ===="
	$(CC) $(CFLAGS) -c $(INCLUDES)/glibc_compat.c -o $(OBJS)/glibc_compat.o $(IFLAGS)

$(OBJS)/vendor.o: $(INCLUDES)/vendor.c
	@mkdir -p $(OBJS)
	@echo "==== Building vendor module ===="
	$(CC) $(CFLAGS) -c $(INCLUDES)/vendor.c -o $(OBJS)/vendor.o $(IFLAGS)

$(OBJS)/main.o: $(SOURCE_FILES) $(INCLUDES) $(MODULES)/*.inc
	@mkdir -p $(OBJS)
	@echo "==== Building core ($(TAGS)) module ===="
	$(CC) $(CFLAGS) -c $(SOURCE)/main.c -o $(OBJS)/main.o $(IFLAGS) 

$(TARGET_NAME): $(OBJS)/main.o $(OBJS)/vendor.o $(OBJS)/libwren.o $(COMPAT_DEP) $(WREN_LIB)
	@echo "==== Linking DOME ($(TAGS)) ===="
	$(CC) $(CFLAGS) $(FFLAGS) -o $(TARGET_NAME) $(OBJS)/*.o $(ICON_OBJECT_FILE) $(LDFLAGS) 
	./scripts/set-executable-path.sh $(TARGET_NAME)
	@echo "DOME built as $(TARGET_NAME)"

$(OBJS):
	mkdir -p $(OBJS)

$(OBJS)/wren.o: $(OBJS)	
	./scripts/setup_wren_web.sh

# EMCC_FLAGS=--profiling -g 
EMCC_FLAGS=""
dome.html: $(SOURCE)/main.c $(MODULES)/*.inc $(INCLUDES)/vendor.c $(OBJS)/wren.o
	emcc -O3 -c include/vendor.c -o $(OBJS)/vendor.o -s USE_SDL=2 -Iinclude $(EMCC_FLAGS)
	emcc -O3 -c src/main.c -o $(OBJS)/main.o -s USE_SDL=2 -Iinclude $(DOME_OPTS) $(EMCC_FLAGS)
	emcc -O1 $(OBJS)/*.o -o dome.html -s USE_SDL=2 -s ALLOW_MEMORY_GROWTH=1 -s ASYNCIFY=1 --shell-file assets/shell.html -s SINGLE_FILE=1 $(EMCC_FLAGS)

dome.bin: $(TARGET_NAME)

clean: 
	rm -rf $(TARGET_NAME) $(MODULES)/*.inc
	rm -rf $(OBJS)/*.o
reset:
	git submodule foreach --recursive git clean -xfd

cloc:
	cloc --by-file --force-lang="java",wren --fullpath --not-match-d "font|dSYM" -not-match-f ".inc" src include/dome.h
