TOPNAME = top

WORK_DIR = $(shell pwd)
INC_PATH := $(WORK_DIR)/csrc/include $(INC_PATH)

VERILATOR = verilator
VINC_PATH := $(WORK_DIR)/vsrc/include $(WORK_DIR)/vsrc/templates/generic $(WORK_DIR)/vsrc/templates/specific
VINCFLAGS = $(addprefix -I, $(VINC_PATH))
VERILATOR_CFLAGS += -MMD --build -cc  \
				-O2 --x-assign fast --x-initial fast --noassert --trace --debug \
				$(VINCFLAGS)
				

BUILD_DIR =$(WORK_DIR)/build
OBJ_DIR = $(BUILD_DIR)/obj_dir

BIN = $(BUILD_DIR)/$(TOPNAME)


GDB_ARGS ?= 
GDB_PORT ?= 1234  # 默认 GDB 调试端口


default: $(BIN)

$(shell mkdir -p $(BUILD_DIR))


# project source
VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
CSRCS = $(shell find $(abspath ./csrc/src) -name "*.c" -or -name "*.cc" -or -name "*.cpp")


# rules for verilator
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\"" -mcmodel=large -fPIC -g

$(BIN): $(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE)
	@rm -rf $(OBJ_DIR)
	@$(VERILATOR)  $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ \
		$(addprefix -CFLAGS , $(CXXFLAGS))  \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN)) >/dev/null

all: default

# Command to excute top
IMG ?=

run: $(BIN)
	@echo \# Runing verilator simulation 
	@$(BIN) $(IMG)

gdb: $(BIN)
	@echo "Starting GDB server for Verilator simulation..."
	gdb --args $(BIN) $(IMG)
	
clean:
	rm -rf $(BUILD_DIR)
	rm *.vcd *.view

.PHONY: default all clean run