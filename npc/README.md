# NPC (New Processor Core)

NPC是一个基于RISC-V架构的处理器核心实现，使用Verilog HDL描述，并通过Verilator进行仿真验证。

## 项目特性

### 支持的Tracer功能
- **指令追踪 (ITRACE)**：记录每条执行的指令
- **函数追踪 (FTRACE)**：跟踪函数调用和返回
- **内存追踪 (MTRACE)**：监控内存访问操作

### 差异测试 (Difftest)
- 与参考模拟器（如NEMU）进行指令级对比
- 自动检测寄存器和PC值差异
- 详细的错误报告和调试信息

## 快速开始

### 环境要求
- Verilator 4.0+
- LLVM 11.0+
- GCC/G++ 支持C++14
- GNU Readline库
- ncurses库

### 使用方式一：在am-kernels中运行（推荐）

am-kernels通过`abstract-machine/scripts/platform/npc.mk`自动管理NPC的所有配置，这是最简便的使用方式：

```bash
# 在am-kernels的任意应用中
cd apps/your_app
make ARCH=riscv32e-npc run
```

**平台自动配置**：`npc.mk`会自动处理：
- 传递正确的命令行参数和宏定义
- 通过变量控制自动启用/禁用Tracer功能
- 配置Difftest的参考模型路径
- 设置运行时环境变量

**Tracer自动管理**：在`npc.mk`中通过宏定义实现Tracer开关：
```makefile
# 在npc.mk中定义
MARCO += -DCONFIG_ITRACE=1    # 指令追踪开关
MARCO += -DCONFIG_FTRACE=1    # 函数追踪开关
MARCO += -DCONFIG_MTRACE=1    # 内存追踪开关
```

**Difftest自动管理**：同样通过宏定义控制：
```makefile
# 在npc.mk中定义
MARCO += -DCONFIG_DIFFTEST=1
```

**使用方式**
```bash
# 无需手动指定，构建系统自动处理
make ARCH=riscv32e-npc run
```

**功能开关**：可以通过注释对应的宏定义来关闭功能上述功能。

### 使用方式二：直接运行NPC

当需要手动控制或调试NPC本身时，可以直接运行：

#### 基础运行
```bash
# 编译NPC
make

# 运行内置测试程序（定义在src/memory/paddr.c中的builtin_img)
make run
# 或手动执行
./build/top
```

#### 带参数运行
```bash
# 运行指定程序
./build/top your_program.bin

# 带Difftest功能
./build/top -d /path/to/nemu/build/riscv32-nemu-interpreter-so your_program.bin
```

### 手动启用Tracer功能

直接运行NPC时，Tracer功能需要通过重新编译启用：

```bash
# 重新编译并启用指令追踪
make clean
make MARCO="-DCONFIG_ITRACE"
./build/top your_program.bin

# 启用多个Tracer
make clean
make MARCO="-DCONFIG_ITRACE -DCONFIG_FTRACE -DCONFIG_MTRACE"
./build/top your_program.bin
```

### 手动启用Difftest

1. 编译NEMU为共享库：
```bash
cd /path/to/nemu
make menuconfig  # 选择Build shared library
make clean && make
# 生成：nemu/build/riscv32-nemu-interpreter-so
```

2. 运行NPC时指定参考模型：
```bash
./build/top -d /path/to/nemu/build/riscv32-nemu-interpreter-so your_program.bin
```

## 项目结构

```
npc/
├── vsrc/          # Verilog源代码
│   ├── rtl/       # RTL实现
│   └── include/   # Verilog头文件
├── csrc/          # C++仿真代码
│   ├── src/       # 源代码
│   └── include/   # 头文件
├── script/        # 构建脚本
└── Makefile       # 主Makefile
```

## 调试功能

### 交互式调试器
NPC内置简单的调试器，支持以下命令：
- `help` - 显示帮助信息
- `si [n]` - 单步执行n条指令
- `info r` - 显示寄存器状态
- `x n addr` - 查看内存内容
- `c` - 继续执行
- `q` - 退出

### 差异测试错误报告
当NPC与参考模型不一致时，会显示：
```
gpr[5] should be 0x00000001 but be 0x00000000.
pc should be 0x80000010 but be 0x80000014.
npc: ABORT at pc = 0x80001000
```
