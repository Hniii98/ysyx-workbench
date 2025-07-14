#include <common.h>
#include <verilator.h>
#include <sim.hh>
#include <utils.h>
#include <sim.h>
#include <difftest-def.h>


static Vtop* dut = NULL;
VerilatedContext *contextp = NULL;
VerilatedVcdC* tfp = NULL;
uint32_t g_frozen_pc; // freeze pc value before npc execute once instruction

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

extern "C" uint32_t paddr_read(uint32_t addr, int len);
extern "C" void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);

/* npc excute one cyclye and recore wave */
void npc_exec_once(){
	
	uint32_t inst_val = paddr_read(dut->PC, 4);
	dut->inst = inst_val; // fetch instruction from host simulated memory
	g_frozen_pc = dut->PC; // restore the current pc from top module
	single_cycle();

#ifdef CONFIG_ITRACE
	char *p = g_npc_state.logbuf;
	p += snprintf(p, sizeof(g_npc_state.logbuf), FMT_WORD ":", g_frozen_pc);
	int ilen = 4;
	int i;
	uint8_t *inst = (uint8_t *)&inst_val;
	for (i = ilen - 1; i >= 0; i --) { // little endian
		p += snprintf(p, 4, " %02x", inst[i]);
	}
	int space_len =  1; // fill blank at rear
	memset(p, ' ', space_len);
	p += space_len;

	disassemble(p, g_npc_state.logbuf + sizeof(g_npc_state.logbuf) - p,
		g_frozen_pc, inst, ilen);
	//printf("%s\n", g_npc_state.logbuf);
#endif

#ifdef CONFIG_FTRACE
	bool ftrace_enable = sim_get_is_uncondjump();
	uint32_t dnpc = sim_get_nextpc();
	uint8_t rd = sim_get_rd();

	if(ftrace_enable){
		write_uncondjump_trace(g_frozen_pc, rd, dnpc);
		//ftracedata_display_once();
	}
	
	

#endif	
}

static void trace_and_difftest(){
	/* traece */
	IFDEF(CONFIG_ITRACE, printf("%s\n", g_npc_state.logbuf));
	IFDEF(CONFIG_FTRACE,
		if(sim_get_is_uncondjump()) { ftracedata_display_once(); });

	/* difftest*/
	IFDEF(CONFIG_DIFFTEST, difftest_step());
}

void execute(uint64_t n){
	for(; n > 0; n --){
		npc_exec_once();
		trace_and_difftest();
		if (g_npc_state.state != NPC_RUNNING) break;
	}	
}

void npc_exec(uint64_t n){
	switch (g_npc_state.state) {
		case NPC_END: case NPC_ABORT:
			printf("Program excution has ended. To restart the program, exit NPC and run again \n");
			return;
		default: g_npc_state.state = NPC_RUNNING;
	}

	execute(n);

	switch (g_npc_state.state) {
		case NPC_RUNNING: g_npc_state.state = NPC_STOP; break;

		case NPC_END: case NPC_ABORT:
			printf("npc: %s at pc = " FMT_WORD "\n",
    	    	(g_npc_state.state == NPC_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
      	 		(g_npc_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
         		ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED) )),
          		g_npc_state.halt_pc);
			break;
		/* FALL THROUGH */
		case NPC_QUIT:
			break;
	}	
}



#define NR_SCOPES ARRLEN(scopes_name)

const char* scopes_name [] = {
	"alu", "control", "immgen", "pc", "regfiles",
	"writeback",
};

void npc_set_scope(const char* name){
    const char* prefix = "TOP.top.u_";
    char final_name[128];  

    for(int i = 0 ; i < NR_SCOPES; i++){
        if(strcmp(name, scopes_name[i]) == 0){
            snprintf(final_name, sizeof(final_name), "%s%s", prefix, name);  
            svScope scope = svGetScopeFromName(final_name);
            svSetScope(scope);
            return;
        }
    }

    printf("[npc] Invalid module name '%s' \n", name);
}