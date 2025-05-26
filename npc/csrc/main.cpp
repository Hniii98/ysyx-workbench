#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cstdint>
#include "Vtop.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include "svdpi.h"
#include "Vtop__Dpi.h"


#define MSIZE 0x80000000
#define BASE  0x80000000

static Vtop* dut = NULL;
VerilatedContext* contextp = NULL;
VerilatedVcdC* tfp = NULL;

uint8_t memory[MSIZE] = {0};

static bool loop = true;


uint32_t pmem_read(uint32_t addr);
static void reset(int n);
static void single_cycle();
void step_and_dump_wave();
void sim_init();
void sim_exit();
void end_loop();


int main(int argc, char** argv){
  
    // 指令1: addi x1, x0, 5   (x1 = x0 + 5) (机器码 0x00500093)
  memory[0] = 0x93;  
  memory[1] = 0x00;  
  memory[2] = 0x50;  
  memory[3] = 0x00;  

  // 指令2: addi x3, x2, 0x7F (x3 = x2 + 127) (机器码 0x07F10193)
  memory[4] = 0x93;  
  memory[5] = 0x01;  
  memory[6] = 0xF1; 
  memory[7] = 0x07; 

// 指令3：ebreak (机器码 0x00100073)
  memory[8] = 0x73;  
  memory[9] = 0x00;  
  memory[10]= 0x10; 
  memory[11]= 0x00; 


  //int cmd_cnt = 2;


  sim_init();
  reset(10);
  loop = true;

while(loop){
    dut->inst = pmem_read(dut->pc);
    step_and_dump_wave();

    printf("PC=0x%08X, INST=0x%08X\n", dut->pc, dut->inst);
}

  sim_exit();

  return 0;
}

uint32_t pmem_read(uint32_t addr){
  uint8_t *host = memory + addr - BASE; // compute arry index
  return *(uint32_t *)host; // read 4 bytes
 
  // assert(addr % 4 == 0);
  // assert(addr >= BASE && addr < BASE + MSIZE);

  
  // uint32_t offset = addr - BASE;
  // uint8_t *host = memory + offset;  // 正确指针运算
  // uint32_t val;
  // memcpy(&val, host, sizeof(val));  // 避免严格别名问题
  // return val; 
}

static void single_cycle(){
      // 下降沿
    dut->clk = 0; 
    dut->eval();
    tfp->dump(contextp->time());
    contextp->timeInc(1);
    
    // 上升沿
    dut->clk = 1;
    dut->eval();
    tfp->dump(contextp->time());
    contextp->timeInc(1);
}

static void reset(int n){
  dut->rst = 1;
  while(n--) single_cycle();
  dut->rst = 0;
}

void step_and_dump_wave() {
    single_cycle();
}

void sim_init(){
 
  contextp = new VerilatedContext;
  contextp->traceEverOn(true);  
  dut = new Vtop{contextp};
  
  tfp = new VerilatedVcdC;
  dut->trace(tfp, 99);
  tfp->open("dump.vcd");
}

void sim_exit() {
    
    tfp->close();
    delete dut;
    delete tfp;
    delete contextp;
}

void end_loop(){
  loop = false;
}

