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

  /* Bind pinsn to nvboard */
  nvboard_bind_all_pins(top);
  nvboard_init();

  while(!contextp->gotFinish()){
	top->eval();
	contextp->timeInc(1);
	nvboard_update();
  } 
	

  nvboard_quit();
  delete top;
  delete contextp;

  return 0;
}
