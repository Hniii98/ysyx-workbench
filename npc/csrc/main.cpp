#include "Vtop.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <nvboard.h>


void nvboard_bind_all_pins(Vtop* top);

int main(int argc, char** argv){
  /* Verilator context */
  VerilatedContext *contextp = new VerilatedContext;
  contextp->commandArgs(argc, argv);
  Vtop *top = new Vtop{contextp};

  /* Trace wave */
  Verilated::traceEverOn(true);
  VerilatedVcdC* tfp = new VerilatedVcdC;
  top->trace(tfp, 99);
  tfp->open("waveform.vcd");

  /* Bind pinsn to nvboard */
  nvboard_bind_all_pins(top);
  nvboard_init();

  while(!contextp->gotFinish()){
	//int a = rand() & 1;
	//int b = rand() & 1;
	//top->a = a;
	//top->b = b;
	top->eval();
	tfp->dump(contextp->time());
	contextp->timeInc(1);
//	printf("a = %d, b = %d, f = %d\n", a, b, top->f);
//	assert(top->f == (a ^ b));
	nvboard_update();
  } 
	
  tfp->close();

  nvboard_quit();
  delete top;
  delete contextp;

  return 0;
}
