# Define different top name
CLI_TOPNAME = top
FPGA_TOPNAME = top
LIGHT_TOPNAME = light

# Define differenet nxdc files
FPGA_NXDC_FILES = constr/top.nxdc
LIGHT_NXDC_FILES = constr/light.nxdc

# Verilator config
VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc  \
				-O3 --x-assign fast --trace --x-initial fast --noassert

# Build directories
BUILD_DIR = ./build

# Auto bind
FPGA_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind_fpga.cpp)
LIGHT_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind_light.cpp)

$(FPGA_AUTO_BIND): $(FPGA_NXDC_FILES)
	@python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

$(LIGHT_AUTO_BIND): $(LIGHT_NXDC_FILES)
	@python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

# Source files
FPGA_CSRCS = $(abspath ./csrc/fpga.cpp) $(FPGA_AUTO_BIND)
CLI_CSRCS = $(abspath ./csrc/cli.cpp)
LIGHT_CSRCS = $(abspath ./csrc/light.cpp) $(LIGHT_AUTO_BIND)

FPGA_VSRCS = $(abspath ./vsrc/top.v)
CLI_VSRCS = $(abspath ./vsrc/top.v)
LIGHT_VSRCS = $(abspath ./vsrc/light.v)

# Rules for NVBoard
include $(NVBOARD_HOME)/scripts/nvboard.mk

# Rules for verilator
INC_PATH ?=
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) 

# Create build directory
$(shell mkdir -p $(BUILD_DIR))
# Function to build a specific target

define BUILD_TARGET
$(1)_OBJ_DIR = $(BUILD_DIR)/$(1)_obj_dir
$(1)_BIN = $(BUILD_DIR)/$(1)

$$($(1)_BIN): $$($(1)_VSRCS)  $$($(1)_CSRCS) $(NVBOARD_ARCHIVE)
	@rm -rf $$
	$(VERILATOR) $(VERILATOR_CFLAGS) \
        --top-module $($(1)_TOPNAME) $$^\
        $(addprefix -CFLAGS , $(CXXFLAGS) -DTOP_NAME="\"V$($(1)_TOPNAME)\"") \
        $(addprefix -LDFLAGS , $(LDFLAGS)) \
        --Mdir $$($(1)_OBJ_DIR) --exe -o $$(abspath $$($(1)_BIN))

.PHONY: $(1)_top
$(1)_top: $$($(1)_BIN)
endef

sim: FPGA_top CLI_top LIGHT_top 
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!

# Build targets
$(eval $(call BUILD_TARGET,FPGA))
$(eval $(call BUILD_TARGET,CLI,))
$(eval $(call BUILD_TARGET,LIGHT))

# Default target
default:  sim

# Clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: default clean sim FPGA_top CLI_top LIGHT_top
