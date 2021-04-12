# Paths
SOURCE=src
LIBS=lib
OBJS=obj
INCLUDES=include
SOURCE_FILES = $(shell find src -type f)
UTILS = $(SOURCE)/util
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
TAGS = $(ARCH) $(SYSTEM) $(MODE) $(FRAMEWORK) $(SYMBOLS)

ifneq ($(filter debug,$(TAGS)),)
TAGS += symbols
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

SDL_CONFIG ?= $(shell which sdl2-config 1>/dev/null && echo "sdl2-config" || (which "$(LIBS)/sdl2-config" 1>/dev/null && echo "$(LIBS)/sdl2-config" || echo ""))

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
ifneq ($(filter macosx,$(TAGS)),)
CFLAGS += -mmacosx-version-min=10.12
endif

ifneq ($(filter release,$(TAGS)),)
CFLAGS += -O3
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

ifneq ($(filter release,$(TAGS)),)
DEPS += -lwren
else ifneq ($(filter debug,$(TAGS)),)
DEPS += -lwrend
endif
ifneq ($(and $(filter windows,$(TAGS)),$(filter static,$(TAGS))),)
WINDOW_MODE ?= windows
WINDOW_MODE_FLAG = -m$(WINDOW_MODE)
STATIC_FLAG += -static

else ifneq ($(and $(filter macosx,$(TAGS)), $(filter framework,$(TAGS)), $(filter shared,$(TAGS))),)
FFLAGS += -F/Library/Frameworks -framework SDL2
endif

LDFLAGS = -L$(LIBS) $(WINDOW_MODE_FLAG) $(SDLFLAGS) $(STATIC_FLAG)
ifneq ($(filter linux,$(TAGS)),)
	COMPAT_DEP = $(OBJS)/glibc_compat.o
	LDFLAGS += -Wl,--wrap=log,--wrap=log2,--wrap=exp,--wrap=pow,--wrap=expf,--wrap=powf,--wrap=logf
endif
LDFLAGS += $(DEPS)




# Build Rules
PROJECTS := dome.bin
.PHONY: all clean reset cloc $(PROJECTS)

all: $(PROJECTS)

WREN_LIB ?= $(LIBS)/libwren.a
WREN_PARAMS ?= $(ARCH) WREN_OPT_RANDOM=0 WREN_OPT_META=1   
$(LIBS)/wren/lib/libwren.a:
	@echo "==== Cloning Wren ===="
	git submodule update --init -- $(LIBS)/wren
$(LIBS)/wren: $(LIBS)/wren/lib/libwren.a
$(WREN_LIB): $(LIBS)/wren
	@echo "==== Building Wren ===="
	./scripts/setup_wren.sh $(WREN_PARAMS)

$(MODULES)/*.inc: $(UTILS)/embed.c $(MODULES)/*.wren
	@echo "==== Building DOME modules  ===="
	./scripts/generateEmbedModules.sh

$(OBJS)/glibc_compat.o: $(INCLUDES)/glibc_compat.c
	@mkdir -p $(OBJS)
	@echo "==== Building glibc_compat module ===="
	$(CC) $(CFLAGS) -c $(INCLUDES)/glibc_compat.c -o $(OBJS)/glibc_compat.o $(IFLAGS)

$(OBJS)/vendor.o: $(INCLUDES)/vendor.c
	@mkdir -p $(OBJS)
	@echo "==== Building vendor module ===="
	$(CC) $(CFLAGS) -c $(INCLUDES)/vendor.c -o $(OBJS)/vendor.o $(IFLAGS)

$(OBJS)/main.o: $(SOURCE_FILES) $(INCLUDES) $(WREN_LIB) $(MODULES)/*.inc
	@mkdir -p $(OBJS)
	@echo "==== Building core ($(TAGS)) module ===="
	$(CC) $(CFLAGS) -c $(SOURCE)/main.c -o $(OBJS)/main.o $(IFLAGS) 

$(TARGET_NAME): $(OBJS)/main.o $(OBJS)/vendor.o $(COMPAT_DEP) $(WREN_LIB)
	@echo "==== Linking DOME ($(TAGS)) ===="
	$(CC) $(CFLAGS) $(FFLAGS) -o $(TARGET_NAME) $(OBJS)/*.o $(ICON_OBJECT_FILE) $(LDFLAGS) 
	./scripts/set-executable-path.sh $(TARGET_NAME)
	@echo "DOME built as $(TARGET_NAME)"

dome.bin: $(TARGET_NAME)

clean: 
	rm -rf $(TARGET_NAME) $(MODULES)/*.inc
	rm -rf $(OBJS)/*.o
reset:
	git submodule foreach --recursive git clean -xfd
	rm -rf $(LIBS)/libwren.a 
	rm -rf $(LIBS)/libwrend.a
	rm -rf $(INCLUDES)/wren.h

cloc:
	cloc --by-file --force-lang="java",wren --fullpath --not-match-d "util" -not-match-f ".inc" src
