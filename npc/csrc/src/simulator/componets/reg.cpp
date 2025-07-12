#include <macro.h>
#include <sim.h>
#include <sim.hh>
#include <verilator.h>
#include <dpic_impl.h>


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