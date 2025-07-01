#include <common.h>

uint32_t *reg_ptr = NULL;
const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};






extern "C" void set_gpr_ptr(uint32_t *ptr){
	reg_ptr = ptr;
	assert(reg_ptr);
}

uint32_t get_reg_rawdata(uint32_t index){
	assert(index >=0 && index <=31);
	return reg_ptr[index];
}

#define NR_REGS ARRLEN(regs)
void npc_reg_display(){
	if(NR_REGS == 0){	printf("Empty regs array");}
	/* display regfiles value in hex and unsigned dec types*/
	for(int idx = 0; idx < NR_REGS; idx++){
		printf("%-20sOx%-18x%-20u", regs[idx]
								  , get_reg_rawdata(idx)	 // hex
								  , get_reg_rawdata(idx));   // unsigned dec
		printf("\n");
	}
}

