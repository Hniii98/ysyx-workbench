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
#include <isa-def.h>

typedef struct {
  word_t mstatus;
  word_t mtvec;
  word_t mepc;
  word_t mcause; 
} SystemRegs;

static SystemRegs CSR = {
  .mstatus = 0x1800,
  .mtvec = 0,
  .mepc = 0,
  .mcause = 0,
};


word_t csr_read(uint32_t id) {
  switch (id) {
    case 0x300: return CSR.mstatus; // mstatus
    case 0x305: return CSR.mtvec;   // mtvec
    case 0x341: return CSR.mepc;    // mepc
    case 0x342: return CSR.mcause;  // mcause
    default: panic("Unknown CSR read: 0x%x", id);
  }
}


void csr_write(uint32_t id, word_t val) {
  switch (id) {
    case 0x300: CSR.mstatus = val; break;
    case 0x305: CSR.mtvec = val; break;
    case 0x341: CSR.mepc = val; break;
    case 0x342: CSR.mcause = val; break;
    default: panic("Unknown CSR write: 0x%x", id);
  }
}

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  csr_write(MEPC, epc);
  csr_write(MCAUSE, NO);
  return csr_read(MTVEC);
}




word_t isa_query_intr() {
  return INTR_EMPTY;
}
