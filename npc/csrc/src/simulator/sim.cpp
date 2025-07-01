#include <common.h>
#include <verilator.h>

static Vtop* dut = NULL;
VerilatedContext *contextp = NULL;
VerilatedVcdC* tfp = NULL;



void sim_init(){
	printf("[NPC] simulator initialization ...\n");
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

// void step_and_dump_wave() {
//     single_cycle();
// }

void npc_init(){
	/* initial global parameters*/
	sim_init();

	/* reset all unit of npc */
	reset(10);
}

uint32_t paddr_read(uint32_t addr, int len);

/* npc excute one cyclye and recore wave */
void npc_exec_once(){
	printf("[NPC] fectch instructions at" FMT_PADDR " and excute.\n", dut->PC);
	dut->inst = paddr_read(dut->PC, 4); // read 4 bytes instructions
	single_cycle();
}

void execute(uint64_t n){
	for(; n > 0; n --){
		npc_exec_once();
		if (npc_state.state != NPC_RUNNING) break;
	}	
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
			printf("nemu: %s at pc = " FMT_WORD,
    	    	(npc_state.state == NPC_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
      	 		(npc_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
         		ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          		npc_state.halt_pc);
	}	
}