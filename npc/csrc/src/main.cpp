#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cstdint>
#include "Vtop.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include "Vtop__Dpi.h"
#include <assert.h>
#include <getopt.h>


#define MSIZE 0x80000000
#define BASE  0x80000000

static Vtop* dut = NULL;
VerilatedContext* contextp = NULL;
VerilatedVcdC* tfp = NULL;

uint8_t memory[MSIZE];

static const uint32_t builtin_img [] = {
  0x00500093, // addi x1, x0, 5   (x1 = x0 + 5)
  0x07F10193, // addi x3, x2, 0x7F (x3 = x2 + 127)
  0x00100073, // ebreak
};

static bool loop = true;


uint32_t pmem_read(uint32_t addr);
static void reset(int n);
static void single_cycle();
void step_and_dump_wave();
void sim_init();
void sim_exit();
void end_loop();
static long load_img();
static int parse_argument(int argc, char *argv[]);


static char* img_file = NULL;




int main(int argc, char** argv){

  memset(memory, 0, sizeof(memory));
  parse_argument(argc, argv);
  load_img();
  sim_init();
  reset(10);
  loop = true;
  int loop_times = 12;
 

  while(loop){
      printf("-----------------------------------\n");
     
      dut->inst = pmem_read(dut->PC);  
      
      step_and_dump_wave();
      
      printf("-----------------------------------\n");
    
  }

  sim_exit();

  return 0;
}

uint32_t pmem_read(uint32_t addr){
  assert(addr != 0);
  printf("[memory read]: reading memory at address 0x%x\n", addr);
  uint8_t *host = memory + addr - BASE; // compute arry index
  return *(uint32_t *)host; // read 4 bytes
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
 
  printf("Starting sim init...\n");
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

extern "C" void end_loop() {
  loop = false;
}


static int parse_argument(int argc, char *argv[]){
  if(argc == 1) return -1;
  printf("argv[1]: %s\n", argv[1]);
  img_file = argv[1];
  return 0;
}

static long load_img(){

  if(img_file == NULL){
    printf("Load built-in img to npc \n");
    memcpy(memory, builtin_img, sizeof(builtin_img));
    printf("Writing from address: %p...\n", memory);
    printf("Writing size: %zu...\n", sizeof(builtin_img));
    return sizeof(builtin_img);
  }
  printf("Reading img_file:%s\n", img_file);
  FILE *fp = fopen(img_file, "rb");
  assert(fp);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  printf("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(memory, size, 1, fp);
  assert(ret == 1);

  fclose(fp);
  return size;
}
