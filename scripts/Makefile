# 
# Build flags
ARCH ?= 64bit
SYSTEM ?= 
STATIC ?= 0
TAGS = $(ARCH) $(SYSTEM) $(STATIC)

ifndef verbose
  SILENT = @
endif

PROJECTS := wren dome
.PHONY: all clean reset cloc $(PROJECTS)

all: $(PROJECTS)



WREN_PARAMS ?= 64bit WREN_OPT_RANDOM=1 WREN_OPT_META=1   
wren:                       
	@echo "==== Building wren ===="
	./scripts/setup_wren.sh $(WREN_PARAMS)
