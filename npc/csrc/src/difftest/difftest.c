#include <common.h>
#include <dlfcn.h>
#include <difftest-def.h>
#include <paddr.h>
#include <sim.h>

#ifdef CONFIG_DIFFTEST

void (*ref_difftest_memcpy)(uint32_t addr, void *buf, size_t n, bool direction) = NULL;
void (*ref_difftest_regcpy)(void *dut, bool direction) = NULL;
void (*ref_difftest_exec)(uint64_t n) = NULL;

void (*ref_difftest_init)(int port) = NULL;

/* align ref's status to npc */
static void difftest_align_status(long img_size){
	/* get dut init regs status */
	CPU_state dut_cpu_state;
	sim_send_initstate(&dut_cpu_state);

	/* align ref's regs and memory to dut */
	ref_difftest_regcpy(&dut_cpu_state, DIFFTEST_TO_REF); // paste cpu_state to ref in init status
	ref_difftest_memcpy(RESET_VECTOR, guest_to_host(RESET_VECTOR), img_size, DIFFTEST_TO_REF); // paste img to ref
}

void init_difftest(const char* ref_so_file, long img_size, int port){
	
	assert( ref_so_file != NULL);

	void *handle;
	handle = dlopen(ref_so_file, RTLD_LAZY);
	assert(handle);

	ref_difftest_memcpy = dlsym(handle, "difftest_memcpy");
	assert(ref_difftest_memcpy);
	ref_difftest_regcpy = dlsym(handle, "difftest_regcpy");
	assert(ref_difftest_regcpy);
	ref_difftest_exec = dlsym(handle, "difftest_exec");
	assert(ref_difftest_exec);

	ref_difftest_init = dlsym(handle, "difftest_init");
  	assert(ref_difftest_init);

	printf("Differential testing: %s\n", ANSI_FMT("ON", ANSI_FG_GREEN));
	printf("The result of every instruction will be compared with %s. \n"
      "This will help you a lot for debugging, but also significantly reduce the performance. \n"
      "If it is not necessary, you can turn it off in npc.mk .\n", ref_so_file);

	ref_difftest_init(port);
	difftest_align_status(img_size);
}

/* difftest is performed after the NPC executes one instruction, 
so the current PC value should be interpreted as the next PC, regardless 
of whether it comes from a static next PC or a dynamic next PC. */
static void checkregs(uint32_t *ref_gpr, uint32_t ref_next_pc) {
  /* dut and ref are not in same status, change npc_state */
  if (!sim_difftest_checkregs(ref_gpr, ref_next_pc)) {
    g_npc_state.state = NPC_ABORT;
	g_npc_state.halt_pc = g_frozen_pc;
  }
  
}

void difftest_step(){
	CPU_state ref_cpu_state;
	
	/* to simplify it, we only check regs in difftest function. cause diff in 
	   memory of dut and ret will spread to regs in a certain time which we can 
	   detect. */
	ref_difftest_exec(1);
	ref_difftest_regcpy(&ref_cpu_state, DIFFTEST_TO_DUT); // copy cpu_state after excuting once from ref 
	checkregs(ref_cpu_state.gpr,  ref_cpu_state.pc); 
}


#endif