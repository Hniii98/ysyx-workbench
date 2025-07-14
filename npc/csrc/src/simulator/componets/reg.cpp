#include <macro.h>
#include <sim.h>
#include <sim.hh>
#include <verilator.h>
#include <dpic_impl.h>
#include <utils.h>
#include <difftest-def.h>


#define NR_REGS ARRLEN(regs_name)

const char *regs_name[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

uint32_t sim_get_regval(uint32_t index){
    npc_set_scope("regfiles"); // Set npc scope to regfiles.
    return npc_send_gprval(index);
}


extern "C" void sim_display_regs() {
    for (int i = 0; i < NR_REGS; i++) {
        uint32_t val = sim_get_regval(i);
        printf("%-5d %-8s 0x%-18x %-20u", 
               i, regs_name[i], val, val);
        if (i == 0 && val != 0) {
            printf(" (Warning: x0 should be zero!)");
        }
        printf("\n");
    }
   
}

extern "C" bool sim_difftest_checkregs(uint32_t *ref_gpr, uint32_t ref_next_pc){
    for(int i = 0; i < NR_REGS; i++){
        uint32_t dut = sim_get_regval(i);
        uint32_t ref = ref_gpr[i]; // 
        if(ref != dut) return false;
    }
    uint32_t dut_next_pc = sim_get_nextpc();

     
    if(ref_next_pc != dut_next_pc) return false;

    return true;
}

#include <paddr.h>
/* get npc init state*/
extern "C" void sim_send_initstate(CPU_state *dut_state){
    for(int i = 0; i < NR_REGS; i++){
        uint32_t dut = sim_get_regval(i);
        dut_state->gpr[i] = dut;
    }

    dut_state->pc = RESET_VECTOR;
}