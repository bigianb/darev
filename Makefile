
# ------------

DEBUG ?= 0

BASENAME = BGDA_123

# OPL needs this to look like a sony binary
EE_BIN = fs/$(BASENAME).00
EE_SRC_DIR = src/
EE_SRC_AD_DIR = src/animDebug/
EE_SRC_PS2_DIR = src/ps2/

EE_OBJS_DIR = obj/
EE_OBJS_AD_DIR = obj/animDebug/
EE_OBJS_PS2_DIR = obj/ps2/

MAPFILE = fs/$(BASENAME).MAP
EE_LDFLAGS += -Wl,-Map,$(MAPFILE)

EE_LIBS = -lpad -lgs -ldma -lc

ifeq ($(DEBUG),1)
  EE_OPTFLAGS = -O0
  EE_CFLAGS += -D__DEBUG -g
else
  EE_OPTFLAGS = -O2
endif

MAIN_OBJS = 
PS2_OBJS =	display dlist draw elfData filesys font frameFunctions \
		GIFTag gsAllocator \
		levelLoop lump main menu pad scene showLanguageMenu state \
		TexDecoder text texture trace
ANIMDEBUG_OBJS = animDebugSetup

MAIN_OBJS := $(MAIN_OBJS:%=$(EE_OBJS_DIR)%.o)
ANIMDEBUG_OBJS := $(ANIMDEBUG_OBJS:%=$(EE_OBJS_AD_DIR)%.o)
PS2_OBJS := $(PS2_OBJS:%=$(EE_OBJS_PS2_DIR)%.o)

# Generate .d files to track header file dependencies of each object file
EE_CFLAGS += -MMD -MP -Isrc -Isrc/ps2/hw
EE_OBJS += $(MAIN_OBJS) $(PS2_OBJS) $(ANIMDEBUG_OBJS)
EE_DEPS = $($(filter %.o,$(EE_OBJS)):%.o=%.d)

.PHONY: all release debug rebuild iso elf

all: $(EE_BIN) iso

elf: $(EE_BIN)

debug:
	$(MAKE) DEBUG=1 all

clean:
	echo "Cleaning..."
	rm -fr $(MAPFILE) $(EE_BIN) $(EE_OBJS_DIR) $(BASENAME).ISO

rebuild: clean all

iso: $(BIN_TARGET)
	mkisofs -o $(BASENAME).ISO ./fs

$(EE_OBJS_DIR):
	@mkdir -p $(EE_OBJS_PS2_DIR)
	@mkdir -p $(EE_OBJS_AD_DIR)

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.cpp | $(EE_OBJS_DIR)
	$(EE_CXX) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_AD_DIR)%.o: $(EE_SRC_AD_DIR)%.cpp | $(EE_OBJS_DIR)
	$(EE_CXX) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_PS2_DIR)%.o: $(EE_SRC_PS2_DIR)%.cpp | $(EE_OBJS_DIR)
	$(EE_CXX) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@


ifndef PS2SDK
ps2sdk-not-setup:
	@echo "PS2SDK is not setup. Please setup PS2SDK before building this project"
endif

include make/Makefile.pref
include make/Makefile.eeglobal_cpp

-include $(EE_DEPS)
