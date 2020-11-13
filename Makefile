# Paths
SRC=src
LIBS=$(SRC)/lib
INCLUDES=$(SRC)/includes
MODULES=$(SRC)/modules
SCRIPTS=scripts

# Build flags
ARCH ?= 64bit
SYSTEM ?= 
STATIC ?= 0
TAGS = $(ARCH) $(SYSTEM) $(STATIC)

ifndef verbose
  SILENT = @
endif

WREN_LIB ?= $(LIBS)/libwren.a

PROJECTS := dome wren
.PHONY: all clean reset cloc $(PROJECTS)

all: $(PROJECTS)



WREN_PARAMS ?= 64bit WREN_OPT_RANDOM=1 WREN_OPT_META=1   
$(LIBS)/wren/lib/libwren.a:
	git submodule update --init -- $(LIBS)/wren
$(LIBS)/wren: $(LIBS)/wren/lib/libwren.a
$(WREN_LIB): $(LIBS)/wren
	@echo "==== Building wren ===="
	./scripts/setup_wren.sh $(WREN_PARAMS)
wren: $(WREN_LIB)

clean: 
reset:
	git submodule foreach --recursive git clean -xfd
	rm -rf $(LIBS)/libwren.a 
	rm -rf $(LIBS)/libwren_d.a
	rm -rf $(INCLUDES)/wren.h 
