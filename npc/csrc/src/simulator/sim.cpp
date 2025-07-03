#include <common.h>
#include <verilator.h>

static Vtop* dut = NULL;
VerilatedContext *contextp = NULL;
VerilatedVcdC* tfp = NULL;
uint32_t current_pc;



void param_init(){
	printf("[npc] simulator initialization ...\n");
	contextp = new VerilatedContext;
	contextp->traceEverOn(true);
	dut = new Vtop{contextp};
	tfp = new VerilatedVcdC;
	dut->trace(tfp, 99);
	tfp->open("dump.vcd");
}

void sim_exit(){
	tfp->close();
	delete dut;
	delete tfp;
	delete contextp;
}

void single_cycle(){
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

void reset(int n){
  dut->rst = 1;
  while(n--) single_cycle();
  dut->rst = 0;
  
}

void sim_init(){
	/* initial global parameters*/
	param_init();

	/* reset all unit of npc */
	reset(10);
}



uint32_t paddr_read(uint32_t addr, int len);

/* npc excute one cyclye and recore wave */
void npc_exec_once(){
	printf("[npc] fectch instructions at " FMT_PADDR " and excute.\n", dut->PC);
	dut->inst = paddr_read(dut->PC, 4); // fetch instruction
	current_pc = dut->PC; // restore the current pc from top module
	single_cycle();
}

void execute(uint64_t n){
	for(; n > 0; n --){
		npc_exec_once();
		if (npc_state.state != NPC_RUNNING) break;
	}	
}

extern "C" void npc_reach_ret(int code) {
	npc_state.state = NPC_END;
	npc_state.halt_ret = code;
	npc_state.halt_pc = current_pc;
	
}

void npc_exec(uint64_t n){
	switch (npc_state.state) {
		case NPC_END: case NPC_ABORT:
			printf("Program excution has ended. To restart the program, exit NPC and run again \n");
			return;
		default: npc_state.state = NPC_RUNNING;
	}

	execute(n);

	switch (npc_state.state) {
		case NPC_RUNNING: npc_state.state = NPC_STOP; break;

		case NPC_END: case NPC_ABORT:
			printf("npc: %s at pc = " FMT_WORD "\n",
    	    	(npc_state.state == NPC_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
      	 		(npc_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
         		ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED) )),
          		npc_state.halt_pc);
			break;
		/* FALL THROUGH */
		case NPC_QUIT:
			break;
	}	
}