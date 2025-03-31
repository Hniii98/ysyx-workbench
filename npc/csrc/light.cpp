#include "Vlight.h"
#include "verilated.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <nvboard.h>

static Vlight light;
void single_cycle();
void reset(int n);
void nvboard_bind_all_pins(Vlight *light);

int main(int argc, char** argv){

  nvboard_bind_all_pins(&light);
  nvboard_init();

  reset(10); // reset 10 cycle

  while(1){
	nvboard_update();
	single_cycle();
  }
  
  return 0;
}

void single_cycle(){
  light.clk = 0; light.eval();
  light.clk = 1; light.eval();
}

void reset(int n){
  light.rst = 1;
  while(n-- > 0) single_cycle();
  light.rst = 0;
}
