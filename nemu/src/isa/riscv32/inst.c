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

#include "local-include/reg.h"
#include "include/isa-def.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>
#include <trace.h>

/* gpr data type is word_t(uint32_t) */
#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
  TYPE_I, TYPE_U, TYPE_S,
  TYPE_N, TYPE_J, TYPE_B,
  TYPE_R, // none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immJ() do { \
					*imm = SEXT( \
						(BITS(i, 31, 31) << 20) | \
						(BITS(i, 19, 12) << 12) | \
						(BITS(i, 20, 20) << 11) | \
					  (BITS(i, 30, 21) <<  1) \
					, 21); \
} while(0) 
#define immB() do { \
					*imm = SEXT( \
						(BITS(i, 31, 31) << 12) | \
						(BITS(i,  7,  7) << 11) | \
						(BITS(i, 30, 25) <<  5) | \
						(BITS(i, 11,  8) <<  1)   \
					, 13); \
} while(0)


static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
	  case TYPE_S: src1R(); src2R(); immS(); break;
	  case TYPE_J:                   immJ(); break;
	  case TYPE_B: src1R(); src2R(); immB(); break;
    case TYPE_R: src1R(); src2R();       ; break;
  }
}

static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  INSTPAT_START();

  /* TYPE_I */
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr  , I, 
	  R(rd) = s->pc + 4; 
    s->dnpc = (src1 + imm) & ~1 
     #ifdef CONFIG_FTRACE
      ; // close previous code line
      void isa_unconjump_trace(vaddr_t pc, int rd, vaddr_t target);
      isa_unconjump_trace(s->pc, rd, s->dnpc) // marco INSTPAT_MATCH add ';' for this code line
    #endif
  );

  /*  parameter len in Mr means bytes, while in SEXT means bits */
  /* using SEXT marco to implement sign-extend */
  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb    , I, R(rd) =	SEXT(Mr(src1 + imm, 1), 8)); 
  /* using implict conversion to implement zero extends */
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu   , I, R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh    , I, R(rd) = SEXT(Mr(src1 + imm, 2), 16));
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu	  , I, R(rd) = Mr(src1 + imm, 2));
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw    , I, R(rd) = Mr(src1 + imm, 4));

  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi  , I, R(rd) = src1 + imm); 
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti  , I, 
	  if((int32_t)src1 < (int32_t)imm)  R(rd) = 1; else  R(rd) = 0); 
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu , I, 
	  if((uint32_t)src1 < (uint32_t)imm)  R(rd) = 1; else  R(rd) = 0); 

  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori  , I, R(rd) = src1 ^ imm);
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori   , I, R(rd) = src1 | imm);
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi  , I, R(rd) = src1 & imm); 
  

  INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli  , I, R(rd) = (uint32_t)src1 << (imm & 0x1F));
  INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli  , I, R(rd) = (uint32_t)src1 >> (imm & 0x1F));
  INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai  , I, R(rd) = (int32_t)src1 >> (imm & 0x1F));  

  
  INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall , I, s->dnpc = isa_raise_intr(11, s->pc));  
  INSTPAT("0011000 00010 00000 000 00000 11100 11", mret  , I, s->dnpc = csr_read(MEPC));
  
  INSTPAT("??????? ????? ????? 001 ????? 11100 11", csrrw , I, 
    word_t temp = csr_read(imm);
    csr_write(imm, src1);
    R(rd) = temp
  );
    
  INSTPAT("??????? ????? ????? 010 ????? 11100 11", csrrs , I,
    word_t temp = csr_read(imm);
    csr_write(imm, temp | src1);
    R(rd) = temp
  );


  /* TYPE_U */
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc , U, R(rd) = s->pc + imm);
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui   , U, R(rd) = imm);

  /* TYPE_S */
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb    , S, Mw(src1 + imm, 1, src2));
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh	  , S, Mw(src1 + imm, 2, src2));
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw    , S, Mw(src1 + imm, 4, src2)); 

  /* TYPE_N */
  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak, N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0

  /* TYPE_J */
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal   , J, 
    R(rd) = s->pc + 4; 
    s->dnpc = s->pc + imm 
    #ifdef CONFIG_FTRACE
      ; // close previous code
      void isa_unconjump_trace(vaddr_t pc, int rd, vaddr_t target);
      isa_unconjump_trace(s->pc, rd, s->dnpc) // marco INSTPAT_MATCH add ';' for this code line
    #endif
  );
  
  /* TYPE_R */
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add   , R, R(rd) = src1 + src2); 
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub   , R, R(rd) = src1 - src2); 

  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll   , R, R(rd)  = src1 << (src2 & 0x1F)); 
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt   , R,
	  if((int32_t)src1 < (int32_t)src2) R(rd) = 1; else R(rd) = 0); 
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu  , R,
	  if((uint32_t)src1 < (uint32_t)src2) R(rd) = 1; else R(rd) = 0); 


  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor   , R, R(rd) = src1 ^ src2);
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl   , R, R(rd) = src1 >> (src2 & 0x1F));
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra   , R, R(rd) = (int32_t)src1 >> (src2 & 0x1F));
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11",  or   , R, R(rd) = src1 | src2);
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and   , R, R(rd) = src1 & src2);

  INSTPAT("0000001 ????? ????? 000 ????? 01100 11",  mul  , R, R(rd) = src1 * src2);
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh  , R,
    int64_t result = (int64_t)(int32_t)src1 * (int64_t)(int32_t)src2;
    R(rd) = (uint32_t)(result >> 32)
  );
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11",  mulhu  , R, 
    uint64_t result = (uint64_t)src1 * (uint64_t)src2;
    R(rd) = (uint32_t)(result >> 32)
  );
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11",  div  , R, 
    do {
        if (src2 == 0) {
            R(rd) = -1; // divide zero
        } else if ((int32_t)src1 == INT32_MIN && (int32_t)src2 == -1) {
            R(rd) = INT32_MIN; // overflow
        } else {
            R(rd) = (int32_t)src1 / (int32_t)src2;
        }
    } while (0)
  );
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu  , R, 
    do {
        if (src2 == 0) {
            R(rd) = UINT32_MAX; // divide zero
        } else {
            R(rd) = src1 / src2;
        }
    } while (0)
  );
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11",  rem  , R, 
    do {
        if (src2 == 0) {
            R(rd) = src1; // divide zero
        } else {
            R(rd) = (int32_t)src1 % (int32_t)src2; // operator '%' behavior matches RISCV
        }
    } while (0)
  );
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu  , R, 
    do {
        if (src2 == 0) R(rd) = src1; 
        else R(rd) = src1 % src2;
    } while (0)
  );

  /* TYPE_B */
  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq   , B, 
	  if(src1 == src2) s->dnpc = s->pc + (int32_t)imm); 
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne   , B,
    if(src1 != src2) s->dnpc = s->pc + (int32_t)imm); 
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt   , B, 
    if((int32_t)src1 < (int32_t)src2) s->dnpc = s->pc + (int32_t)imm); 
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   , B, 
    if((uint32_t)src1 < (uint32_t)src2) s->dnpc = s->pc + (int32_t)imm); 
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge   , B, 
    if((int32_t)src1 >= (int32_t)src2) s->dnpc = s->pc + (int32_t)imm); 
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu  , B, 
    if((uint32_t)src1 >= (uint32_t)src2) s->dnpc = s->pc + (int32_t)imm); 

  /* inst is invalid */
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}

#ifdef CONFIG_FTRACE
#include <trace.h>

/* isa unconditional jump target trace */
void isa_unconjump_trace(vaddr_t pc, int rd, vaddr_t target){
  /*  standard calling convention 
      uses 'x1', as well as rd = 1, as return address reg.
      uses 'x5', as well as rd = 5, as an alternate link register. 
      when we do not need back to this funciton , rd = 0, jump will
      be a return. */
  if(rd == 0){
    /* rd = 0, this jump is a return .*/
    ftracedata_write_once(pc, ft_ret, target);
  }
  else{
    /* else, this jump is a funcion call. */
    ftracedata_write_once(pc, ft_call, target);
  }
}
#endif