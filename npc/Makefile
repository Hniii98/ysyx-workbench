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
	-O2 --x-assign fast --x-initial fast --noassert --trace --debug  


VFLAGS += $(VINCFLAGS)

# Build output paths
BIN := $(BUILD_DIR)/$(TOPNAME)

# Collect source files
VSRCS := $(shell find $(abspath ./vsrc) -name "*.v")
CXXSRCS := $(shell find $(abspath ./csrc/src) -name "*.cc" -or -name "*.cpp")


# Compilation flags
CXXFLAGS := -mcmodel=large -fPIC -g $(MARCO)
CXXFLAGS += $(CXXINCFLAGS) 
CXXFLAGS += -DTOP_NAME="\"V$(TOPNAME)\""
CXXFLAGS += $(shell llvm-config --cxxflags) -fPIE


# Link flags
#LDFLAGS := -lreadline -lncurses

# Librarys
LIBS := -lreadline -lncurses
LIBS += $(shell llvm-config --libs)  




# GDB / run args
IMG ?=  # support to run without IMG
ARGS ?=
GDB_PORT ?= 1234

# Compile binary with verilator
# Verilator only support --CFLAGS, so prefix only can be that.
$(BIN): $(VSRCS) $(CXXSRCS) $(COBJS)
	@$(VERILATOR) $(VFLAGS) \
		--top-module $(TOPNAME) $(VSRCS) $(CXXSRCS) \
		$(addprefix --CFLAGS , $(CXXFLAGS)) \
		$(addprefix --LDFLAGS , $(LIBS)) \
		--Mdir $(OBJ_DIR) \
		--exe $(CXXSRCS) $(COBJS) \
		-o $(abspath $(BIN)) > /dev/null 

# Shortcuts
all: default

run: $(BIN)
	@echo \# Running verilator simulation
	@$(BIN) $(ARGS) $(IMG)

gdb: $(BIN)
	@echo "Starting GDB server for Verilator simulation..."
	gdb --args $(BIN) $(ARGS) $(IMG)

clean:
	rm -rf $(BUILD_DIR)
	rm -f *.vcd *.view
	rm -rf obj_dir

print-%:
	@echo '$*=$($*)'

.PHONY: default all clean run gdb

# Default target
default: $(BIN)