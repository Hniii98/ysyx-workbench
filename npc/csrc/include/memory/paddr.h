#ifndef __PADDR_H__
#define __PADDR_H__

#include <common.h>

/* marco defination */
#define RESET_VECTOR 0x80000000
#define CONFIG_MSIZE 0x80000000
#define CONFIG_MBASE 0x80000000


/* variable defination */

/* default img loaded to memory when img_file was not given */
const uint32_t builtin_img [] = {
  0x00500093, // addi x1, x0, 5   (x1 = x0 + 5)
  0x07F10193, // addi x3, x2, 0x7F (x3 = x2 + 127)
  0x00100073, // ebreak
};

/* function defination */
inline bool in_pmem(uint32_t addr) {
	return addr - CONFIG_MBASE < CONFIG_MSIZE;
}


/* decalaration */
uint8_t* guest_to_host(uint32_t paddr);
void init_mem();
void out_of_bound(uint32_t paddr);

#endif