# Set dir
WORK_DIR = $(shell pwd)
BUILD_DIR := $(WORK_DIR)/build
$(shell mkdir -p $(BUILD_DIR))
OBJ_DIR   := $(BUILD_DIR)/obj_dir
SRC_DIR   := $(WORK_DIR)/csrc/src

# Src and ojb files
CSRCS = $(shell find $(SRC_DIR) -name "*.c" )
COBJS := $(CSRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Compilation flags
CINC_PATH := \
  $(WORK_DIR)/csrc/include/

  
CINCFLAGS := $(addprefix -I, $(CINC_PATH))

CFLAGS := -std=gnu11 -mcmodel=large -fPIC -g -MMD -MP
CFLAGS += $(CINCFLAGS)

# Compile rules
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Incremental compilation
DEPS := $(COBJS:.o=.d)
-include $(DEPS)

print-%:
	@echo '$*=$($*)'

.PHONY: all
all: $(COBJS)
