AM_SRCS := riscv/npc/start.S \
           riscv/npc/trm.c \
           riscv/npc/ioe.c \
           riscv/npc/timer.c \
           riscv/npc/input.c \
           riscv/npc/cte.c \
           riscv/npc/trap.S \
           platform/dummy/vme.c \
           platform/dummy/mpe.c

CFLAGS    += -fdata-sections -ffunction-sections
LDFLAGS   += -T $(AM_HOME)/scripts/linker.ld \
						 --defsym=_pmem_start=0x80000000 --defsym=_entry_offset=0x0
LDFLAGS   += --gc-sections -e _start

REF_SO_PATH := $(NEMU_HOME)/build/riscv32-nemu-interpreter-so

# -e  elf file path to npc.
# -d  .so files of NEMU (compile in nemu with corresponding Kconfig).
# -b  npc execute instructions in batch mode. (if you need step mode, comment it)
NPCFLAGS +=  -e $(IMAGE).elf -d $(REF_SO_PATH) -b
CFLAGS += -DMAINARGS=\"$(mainargs)\"

# Define marco to enable trace and difftest in npc. 
# Set a certain value 1 to make marco IFDEF() valid.
MARCO += -DCONFIG_ITRACE=1 # instructions trace
MARCO += -DCONFIG_FTRACE=1 # function call/ret trace
MARCO += -DCONFIG_MTRACE=1 # memory trace
MARCO += -DCONFIG_DIFFTEST=1 # difftest

.PHONY: $(AM_HOME)/am/src/riscv/npc/trm.c

image: $(IMAGE).elf
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin

run: image
	$(MAKE) -C $(NPC_HOME) run ARGS="$(NPCFLAGS)" IMG=$(IMAGE).bin MARCO="$(MARCO)"

gdb: image
	$(MAKE) -C $(NPC_HOME) gdb ARGS="$(NPCFLAGS)" IMG=$(IMAGE).bin MARCO="$(MARCO)"