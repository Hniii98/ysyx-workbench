#include "Vtop.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char** argv){
  /* Verilator context */
  VerilatedContext *contextp = new VerilatedContext;
  contextp->commandArgs(argc, argv);
  Vtop *top = new Vtop{contextp};

  /* Trace wave */
  Verilated::traceEverOn(true);
  VerilatedVcdC* tfp = new VerilatedVcdC;
  top->trace(tfp, 99);
  mkdir("waves", 0755);
  tfp->open("waves/cli_waveform.vcd");

  while(!contextp->gotFinish()){
	int a = rand() & 1;
	int b = rand() & 1;
	top->a = a;
	top->b = b;
	top->eval();
	tfp->dump(contextp->time());
	contextp->timeInc(1);
	printf("a = %d, b = %d, f = %d\n", a, b, top->f);
	assert(top->f == (a ^ b));
  } 
	
  tfp->close();

  delete top;
  delete contextp;

  return 0;
}
