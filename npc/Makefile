#Include C files compile rules
include script/build.mk

TOPNAME := top

# Include paths
CXXINC_PATH := \
  $(WORK_DIR)/csrc/include/

 
CXXINCFLAGS := $(addprefix -I, $(CXXINC_PATH))

# Verilator include paths
VINC_PATH := \
  $(WORK_DIR)/vsrc/include \
  $(WORK_DIR)/vsrc/templates

VINCFLAGS := $(addprefix -I, $(VINC_PATH))

# Verilator options
VERILATOR := verilator
VFLAGS := -MMD --build -cc \
	-O2 --x-assign fast --x-initial fast --noassert --trace --debug  #--public-flat-rw

VFLAGS += $(VINCFLAGS)

# Build output paths
BIN := $(BUILD_DIR)/$(TOPNAME)

# Collect source files
VSRCS := $(shell find $(abspath ./vsrc) -name "*.v")
CXXSRCS := $(shell find $(abspath ./csrc/src) -name "*.cc" -or -name "*.cpp")


# Compilation flags
CXXFLAGS := -mcmodel=large -fPIC -g
CXXFLAGS += $(CXXINCFLAGS) 
CXXFLAGS += -DTOP_NAME="\"V$(TOPNAME)\""


# Link flags
LDFLAGS := -lreadline -lncurses

# GDB / run args
IMG ?=  # support to run without IMG
GDB_ARGS ?=
GDB_PORT ?= 1234

# Compile binary with verilator
# Verilator only support --CFLAGS, so prefix only can be that.
$(BIN): $(VSRCS) $(CXXSRCS) $(COBJS)
	@$(VERILATOR) $(VFLAGS) \
		--top-module $(TOPNAME) $(VSRCS) $(CXXSRCS) \
		$(addprefix --CFLAGS , $(CXXFLAGS)) \
		$(addprefix --LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) \
		--exe $(CXXSRCS) $(COBJS) \
		-o $(abspath $(BIN)) 

# Shortcuts
all: default

run: $(BIN)
	@echo \# Running verilator simulation
	@$(BIN) $(IMG)

gdb: $(BIN)
	@echo "Starting GDB server for Verilator simulation..."
	gdb --args $(BIN) $(IMG)

clean:
	rm -rf $(BUILD_DIR)
	rm -f *.vcd *.view
	rm -rf obj_dir

print-%:
	@echo '$*=$($*)'

.PHONY: default all clean run gdb

# Default target
default: $(BIN)