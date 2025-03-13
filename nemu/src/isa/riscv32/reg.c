/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

#define NR_REGS ARRLEN(regs)

void isa_reg_display() {
	if(NR_REGS == 0){
		printf("Empty regs array");
	}
	for(int idx = 0; idx < NR_REGS; idx++){
		printf("%-20sOx%-18x%-20u", regs[idx], cpu.gpr[idx], cpu.gpr[idx]);
		printf("\n");
	}
}

word_t isa_reg_str2val(const char *s, bool *success) {
	/* given s is empty */
	if(strlen(s) == 0){
		*success = false;
		return -1;
	}

	for(int idx = 0; idx < NR_REGS; idx++){
		if(strcmp(s, regs[idx]) == 0){
			*success = true;
			return cpu.gpr[idx];
		}
	}

	/* no responsive reg name */
	*success = false;
	return -1;
}

/* for testing function isa_reg_str2val */ 
void test_isa_reg_str2val(){
	bool success = true;
	printf("NR_REGS: %d\n", NR_REGS);
	for(int idx = 0; idx < NR_REGS; idx++){
		success = true;
		if(isa_reg_str2val(regs[idx], &success) == (word_t)cpu.gpr[idx] && success) continue;
		else{
			printf("REGS:%s isa_reg_str2val return wrong value\n", 
					regs[idx]);
			return;
		}
	}
	
	success = true;
	/* input empty string, function should return 0 and set success false */
	if(isa_reg_str2val("", &success) != -1 ||  success){
		printf("Input empty, isa_reg_str2val return wrong value\n");
		return;
	}

	/*input illegal reg name */
	success = true;
	if(isa_reg_str2val("None", &success) != -1  || success){
		printf("Input ilegal reg name, isa_reg_str2val return wrong value\n");
		return;
	}

	printf("isa_reg_str2val function testing finished, everything go well so far :)\n");
}
