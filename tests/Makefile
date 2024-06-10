
# ------------

DEBUG ?= 1

BASENAME = tests_ps2
EE_BIN = $(BASENAME).elf
EE_SRC_DIR = ../src/
EE_SRC_PS2_DIR = ../src/ps2/
EE_TEST_DIR = ./


EE_OBJS_DIR = obj/
EE_OBJS_PS2_DIR = obj/ps2/
EE_OBJS_TEST_DIR = obj/

MAPFILE = $(EE_OBJS_DIR)/$(BASENAME).MAP
EE_LDFLAGS += -Wl,-Map,$(MAPFILE)

EE_LIBS = -lpad -lgs -ldma -lc

ifeq ($(DEBUG),1)
  EE_CFLAGS += -D__DEBUG -g
else
  EE_CFLAGS += -O2
endif

MAIN_OBJS = 
PS2_OBJS = dlist gsAllocator state trace
TEST_OBJS = runtests dlist_test gsalloc_test

MAIN_OBJS := $(MAIN_OBJS:%=$(EE_OBJS_DIR)%.o)
PS2_OBJS := $(PS2_OBJS:%=$(EE_OBJS_PS2_DIR)%.o)
TEST_OBJS := $(TEST_OBJS:%=$(EE_OBJS_TEST_DIR)%.o)

# Generate .d files to track header file dependencies of each object file
EE_CFLAGS += -MMD -MP -Ihost_mocks -I../src -I../src/ps2/hw
EE_OBJS += $(TEST_OBJS) $(MAIN_OBJS) $(PS2_OBJS)
EE_DEPS = $($(filter %.o,$(EE_OBJS)):%.o=%.d)

.PHONY: all

all: $(EE_BIN)

clean:
	echo "Cleaning..."
	rm -fr $(MAPFILE) $(EE_BIN) $(EE_OBJS_DIR) $(BASENAME).ISO

rebuild: clean all

$(EE_OBJS_DIR):
	@mkdir -p $(EE_OBJS_PS2_DIR)

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.cpp | $(EE_OBJS_DIR)
	$(EE_CXX) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_PS2_DIR)%.o: $(EE_SRC_PS2_DIR)%.cpp | $(EE_OBJS_DIR)
	$(EE_CXX) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)%.o: $(EE_TEST_DIR)%.cpp | $(EE_OBJS_DIR)
	$(EE_CXX) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@


ifndef PS2SDK
ps2sdk-not-setup:
	@echo "PS2SDK is not setup. Please setup PS2SDK before building this project"
endif

ifdef PS2SDK
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal_cpp
endif

-include $(EE_DEPS)
