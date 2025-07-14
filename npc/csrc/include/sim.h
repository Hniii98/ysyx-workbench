#ifndef __SIM_H__
#define __SIM_H__


#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <inttypes.h>
#include <difftest-def.h>

void npc_exec(uint64_t n);
void sim_init();
void sim_exit();
void sim_display_regs();

/* program counter (PC) in top module within a cycle:
   before top.eval(): PC   <=======>  g_frozen_pc
   after  top.eval(): PC   <=======>  npc_next_pc  
   npc_next can come from static next pc or dynamic next pc which
   depens on the instrution type */ 
extern uint32_t g_frozen_pc; 



/* reg.cpp interface */
bool sim_difftest_checkregs(uint32_t *ref_gpr, uint32_t ref_next_pc);
void sim_send_initstate(CPU_state *dut_state);

#ifdef __cplusplus
}
#endif


#endif